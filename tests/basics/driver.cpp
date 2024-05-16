#include <crails/router_base.hpp>
#include <iostream>

#undef NDEBUG
#include <cassert>

typedef std::map<std::string, std::string> TestParameters;

struct TestAction
{
  void operator()() const { *run = true; }
  std::shared_ptr<bool> run = std::make_shared<bool>(false);
};

typedef Crails::RouterBase<TestParameters, TestAction> TestRouter;

int main()
{
  // Route matching
  {
    TestRouter     router;
    TestAction     action;
    TestParameters params;
    const TestAction* matched_action;

    router.match("/basic-route", action);

    // Matches simple routes
    matched_action = router.get_action("/basic-route", params);
    assert(matched_action != nullptr);
    (*matched_action)();
    assert(*action.run == true);

    // Doesn't match unmatching routes
    matched_action = router.get_action("/basic-rtuoe", params);
    assert(matched_action == nullptr);
  }

  {
    TestRouter     router;
    TestAction     actionA, actionB;
    TestParameters params;
    const TestAction* matched_action;

    router.match("POST", "/basic-route", actionA);
    router.match("", "/basic-route-no-method", actionB);

    // Filters route according to methods
    matched_action = router.get_action("GET", "/basic-route", params);
    assert(matched_action == nullptr);
    matched_action = router.get_action("POST", "/basic-route", params);
    assert(matched_action != nullptr);
    (*matched_action)();
    assert(*actionA.run == true);

    // A route defined with no methods matches any method
    matched_action = router.get_action("GET", "/basic-route-no-method", params);
    assert(matched_action != nullptr);
    matched_action = router.get_action("POST", "/basic-route-no-method", params);
    assert(matched_action != nullptr);
    (*matched_action)();
    assert(*actionB.run == true);
  }

  // Collect parameters
  {
    TestRouter     router;
    TestAction     action;
    TestParameters params;
    const TestAction* matched_action;

    router.match("/:toto/fragments/:titi/:tutu", action);

    matched_action = router.get_action("/123/fragments/abcd/efgh", params);
    assert(matched_action != nullptr);
    assert(params.find("toto") != params.end());
    assert(params.find("titi") != params.end());
    assert(params.find("tutu") != params.end());
    assert(params["toto"] == "123");
    assert(params["titi"] == "abcd");
    assert(params["tutu"] == "efgh");
  }

  // Scopes
  {
    TestRouter     router;
    TestAction     action1, action2;
    TestParameters params;
    const TestAction* matched_action;

    router.scope("toto", [&]()
    {
      assert(router.get_current_scope() == "/toto");
      router.scope("/titi", [&]()
      {
        assert(router.get_current_scope() == "/toto/titi");
        router.match("/hi", action1);
      });
      assert(router.get_current_scope() == "/toto");
      router.scope("/tutu", [&]()
      {
        assert(router.get_current_scope() == "/toto/tutu");
        router.match("/ho", action2);
      });
    });

    matched_action = router.get_action("/toto/titi/hi", params);
    assert(matched_action != nullptr);
    (*matched_action)();
    assert(*action1.run == true);

    matched_action = router.get_action("/toto/tutu/ho", params);
    assert(matched_action != nullptr);
    (*matched_action)();
    assert(*action2.run == true);
  }

  return 0;
}
