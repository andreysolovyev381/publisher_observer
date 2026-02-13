//
// Created by Andrey Solovyev on 16/02/2023.
//

#include <gtest/gtest.h>

#include "include/observer.hpp"

#include <string>

using namespace culib::patterns;

namespace test_global_values {

	double testValue{0.0};

}//!namespace test_global_values

namespace {

    using event_types = testing::Types<
		int,
		std::string
    >;

    template <typename Event>
    struct InheretingObserver final : public culib::patterns::Observer<Event, double> {

	    Event testEvent;

    	void updateCallback(Event const& event, double const& value) & override {
	    	testEvent = event;
		    test_global_values::testValue = value;
    	}
    };

    template <typename Event>
    struct InheretingObserverFail final : public culib::patterns::Observer<Event, double>
    {};


    template<typename T>
    class BasicsPatternsObserver : public testing::Test {};
    TYPED_TEST_SUITE(BasicsPatternsObserver, event_types);

    int const niceValue {10};
}//!namespace

TYPED_TEST(BasicsPatternsObserver, EventNotAvailableAttachNotOk){
	InheretingObserver<TypeParam> o;
	Publisher<TypeParam, double> p;

	TypeParam someEvent {};

	p.Attach(&o, niceValue, someEvent);

	ASSERT_FALSE(p.hasSubscription(&o, someEvent));
}

TYPED_TEST(BasicsPatternsObserver, EventAvailableAttachOk) {
	InheretingObserver<TypeParam> o;
	Publisher<TypeParam, double> p;

	TypeParam someEvent {};
	
	p.addEvent(someEvent);
	p.Attach(&o, niceValue, someEvent);
	
	ASSERT_TRUE(p.hasSubscription(&o, someEvent));
}

TYPED_TEST(BasicsPatternsObserver, UpdateWithNewValueNotOk_NoEvent) {
	InheretingObserver<TypeParam> o;
	Publisher<TypeParam, double> p;

	TypeParam someEvent {};

	p.Attach(&o, niceValue, someEvent);

	ASSERT_FALSE(p.hasSubscription(&o, someEvent));
	
	p.pushUpdate(someEvent, 12.0);
	ASSERT_FALSE(test_global_values::testValue == 12.0);
	test_global_values::testValue = 0.0;
}

TYPED_TEST(BasicsPatternsObserver, UpdateWithNewValueOk) {
	InheretingObserver<TypeParam> o;
	Publisher<TypeParam, double> p;

	TypeParam someEvent {};
	p.addEvent(someEvent);
	p.Attach(&o, niceValue, someEvent);

	ASSERT_TRUE(p.hasSubscription(&o, someEvent));

	p.pushUpdate(someEvent, 12.0);
	ASSERT_TRUE(test_global_values::testValue == 12.0);
	test_global_values::testValue = 0.0;
}

TYPED_TEST(BasicsPatternsObserver, UnableToAddRepetively) {
    InheretingObserver<TypeParam> o;
    Publisher<TypeParam, double> p;
    
    TypeParam someEvent {};
    p.addEvent(someEvent);
    
    p.Attach(&o, niceValue, someEvent);
    ASSERT_TRUE(p.hasSubscription(&o, someEvent));
    auto const& observers1 {p.getObservers(someEvent)};
    ASSERT_EQ(observers1.size(), 1u);
    ASSERT_EQ(observers1[0], std::pair(niceValue, &o));
    
    p.Attach(&o, niceValue, someEvent);
    ASSERT_TRUE(p.hasSubscription(&o, someEvent));
    auto const& observers2 {p.getObservers(someEvent)};
    ASSERT_EQ(observers2.size(), 1u);
    ASSERT_EQ(observers2[0], std::pair(niceValue, &o));

    p.Attach(&o, niceValue - 1, someEvent);
    ASSERT_TRUE(p.hasSubscription(&o, someEvent));
    auto const& observers3 {p.getObservers(someEvent)};
    ASSERT_EQ(observers3.size(), 1u);
    ASSERT_EQ(observers3[0], std::pair(niceValue, &o));
}

TYPED_TEST(BasicsPatternsObserver, NiceValues_Add) {
    InheretingObserver<TypeParam> o1, o2;
    Publisher<TypeParam, double> p;
    
    TypeParam someEvent {};
    p.addEvent(someEvent);
    p.Attach(&o1, niceValue, someEvent);
    ASSERT_TRUE(p.hasSubscription(&o1, someEvent));
    auto const& observers1 {p.getObservers(someEvent)};
    ASSERT_EQ(observers1.size(), 1u);
    ASSERT_EQ(observers1[0], std::pair(niceValue, &o1));
    
    p.Attach(&o2, niceValue-5, someEvent);
    ASSERT_TRUE(p.hasSubscription(&o2, someEvent));
    auto const& observers2 {p.getObservers(someEvent)};
    ASSERT_EQ(observers2.size(), 2u);
    ASSERT_EQ(observers2[0], std::pair(niceValue-5, &o2));
    ASSERT_EQ(observers2[1], std::pair(niceValue, &o1));
}

TYPED_TEST(BasicsPatternsObserver, NiceValues_Remove) {
    InheretingObserver<TypeParam> o1, o2;
    Publisher<TypeParam, double> p;
    
    TypeParam someEvent {};
    p.addEvent(someEvent);
    p.Attach(&o1, niceValue, someEvent);
    ASSERT_TRUE(p.hasSubscription(&o1, someEvent));
    auto const& observers1 {p.getObservers(someEvent)};
    ASSERT_EQ(observers1.size(), 1u);
    ASSERT_EQ(observers1[0], std::pair(niceValue, &o1));
    
    p.Attach(&o2, niceValue-5, someEvent);
    ASSERT_TRUE(p.hasSubscription(&o2, someEvent));
    auto const& observers2 {p.getObservers(someEvent)};
    ASSERT_EQ(observers2.size(), 2u);
    ASSERT_EQ(observers2[0], std::pair(niceValue-5, &o2));
    ASSERT_EQ(observers2[1], std::pair(niceValue, &o1));
    
    p.Detach(&o2, someEvent);
    ASSERT_FALSE(p.hasSubscription(&o2, someEvent));
    auto const& observers3 {p.getObservers(someEvent)};
    ASSERT_EQ(observers3.size(), 1u);
    ASSERT_EQ(observers3[0], std::pair(niceValue, &o1));
}

//uncomment to fail
TYPED_TEST(BasicsPatternsObserver, NoUpdateMethodImplemented_CompileTimeFailure){
//	Observer<TypeParam, double> o;
//	Publisher<TypeParam, double> p;
	ASSERT_TRUE(true);
}