#ifndef  ROUTER_BASE_HPP
# define ROUTER_BASE_HPP

# include <functional>
# include <algorithm>
# include <vector>
# include <string>
# include <regex>
# include <crails/utils/string.hpp>
# define ROUTER_PARAM_PATTERN "[a-zA-Z0-9:_%&$@#+*';.,!=()-]*"

namespace Crails
{
  template<typename PARAMS, typename ACTION>
  class RouterBase
  {
  public:
    typedef ACTION Action;

    struct Item
    {
      Action                   run;
      std::string              method;
      std::regex               regexp;
      std::vector<std::string> param_names;
      std::string              description;
      short                    priority = 0;
      bool operator<(const Item& item) const { return priority < item.priority; }
    };

    typedef std::vector<Item> Items;

    const Action* get_action(const std::string& uri, PARAMS& query_params) const
    {
      return get_action(std::string(), uri, query_params);
    }

    const Action* get_action(const std::string& method, const std::string& uri, PARAMS& query_params) const
    {
      using namespace std;
      for (const Item& item : routes)
      {
        auto matches = sregex_iterator(uri.begin(), uri.end(), item.regexp);

        if ((item.method == "" || item.method == method) &&
             distance(matches, sregex_iterator()) > 0)
        {
          smatch match = *matches;

          for (size_t i = 1 ; i < match.size() ; ++i)
          {
            std::string param_name  = item.param_names[i - 1];
            std::string param_value = uri.substr(match.position(i), match.length(i));

            query_params[param_name] = param_value;
          }
          return &(item.run);
        }
      }
      return 0;
    }

    RouterBase& match(const std::string& route, Action callback)
    {
      return match("", route, callback);
    }

    RouterBase& match(const std::string& method, const std::string& route, Action callback)
    {
      return match(++incremental_order, method, route, callback);
    }

    RouterBase& match(short priority, const std::string& method, const std::string& route, Action callback)
    {
      Item item;

      item_initialize_regex(item, '^' + full_route(route) + '$');
      item.method      = method;
      item.run         = callback;
      item.description = full_route(route);
      item.priority    = priority;
      routes.push_back(item);
      std::sort(routes.begin(), routes.end());
      return *this;
    }

    void scope(const std::string& path, std::function<void()> callback)
    {
      const auto backup = current_scope;

      current_scope = current_scope + '/' + path;
      callback();
      current_scope = backup;
    }

    std::string get_current_scope() const { return remove_duplicate_characters(current_scope, '/'); }

    std::string description() const
    {
      std::string result;
      for (const Item& item : routes)
        result += std::to_string(item.priority) + '\t' + item.method + '\t' + item.description + '\n';
      return result;
    }

  private:
    void item_initialize_regex(Item& item, const std::string& route)
    {
      using namespace std;
      regex find_params(':' + std::string(ROUTER_PARAM_PATTERN), regex_constants::ECMAScript);
      string regexified_route;
      auto matches = sregex_iterator(route.begin(), route.end(), find_params);
      size_t last_position = 0;

      for (auto it = matches ; it != sregex_iterator() ; ++it)
      {
        smatch match = *it;

        regexified_route += route.substr(last_position, match.position(0) - last_position);
        regexified_route  = regexified_route + '(' + ROUTER_PARAM_PATTERN + ')';
        item.param_names.push_back(route.substr(match.position(0) + 1, match.length(0) - 1));
        last_position = match.position(0) + match.length(0);
      }
      regexified_route += route.substr(last_position);
      item.regexp = regex(regexified_route, regex_constants::ECMAScript | regex_constants::optimize);
    }

    std::string full_route(const std::string& path) const
    {
      return remove_duplicate_characters(current_scope + '/' + path + "/?", '/');
    }

    std::string current_scope;
  public:
    Items routes;
    short incremental_order = 0;
  };
}

#endif
