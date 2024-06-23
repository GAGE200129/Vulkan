#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

#include <Core/src/ioc/Container.hpp>

struct Base
{
    virtual int test() { return 420; }
    virtual ~Base() = default;
};

struct Derived: public Base
{
    int test() override { return 1351; }
};

struct Unknown 
{
    int test() { return 1351; }
};

TEST_CASE("IoC-SimpleResolve")
{
    auto instance = gage::ioc::Container();
    instance.register_factory<Base>([] { return std::make_shared<Derived>(); });
    CHECK(1351 == instance.resolve<Base>()->test());
}

TEST_CASE("IoC-ResolveThrow")
{
    auto instance = gage::ioc::Container();
    CHECK_THROWS(instance.resolve<Base>());
}

struct BaseParam
{
    struct IoCParams {
        int answer_of_universe;
    };

    BaseParam(IoCParams params) : answer_of_universe{params.answer_of_universe} {

    }

    int answer_of_universe;
    virtual int test() { return 420; }
    virtual ~BaseParam() = default;
};


TEST_CASE("IoC-Parameterized")
{
    auto instance = gage::ioc::Container();
    instance.register_factory<BaseParam>([] (BaseParam::IoCParams params)
    {
        return std::make_shared<BaseParam>(params); 
    });

    auto base = instance.resolve<BaseParam>();

    CHECK(base->answer_of_universe == 42);
}