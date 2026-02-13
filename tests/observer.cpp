//
// Created by Andrey Solovyev on 16/02/2023.
//

#include <gtest/gtest.h>
#include "include/observer.hpp"
#include <string>


namespace {

    using event_types = testing::Types<
		int,
		std::string
    >;

    using Value = double;
    namespace test_global_values {

	    Value testValue{0.0};

    }//!namespace test_global_values

    template <typename Event>
    struct InheretingObserver final : public culib::patterns::Observer<Event, Value> {

	    Event testEvent;

    	void updateCallback(Event const& event, Value const& value) & override {
	    	testEvent = event;
		    test_global_values::testValue = value;
    	}
    };

    template <typename Event>
    struct InheretingPublisher final : public culib::patterns::Publisher<Event, Value>{};

    template<typename T>
    class BasicsPatternsObserver : public testing::Test {};
    TYPED_TEST_SUITE(BasicsPatternsObserver, event_types);

    int const niceValue {10};
    
}//!namespace


TYPED_TEST(BasicsPatternsObserver, EventNotAvailableAttachNotOk){
    using Event = TypeParam;
	InheretingObserver<Event> o;
	InheretingPublisher<Event> p;

	Event someEvent {};

	// p.addEvent(someEvent);
	p.Attach(&o, niceValue, someEvent);

	ASSERT_FALSE(p.hasSubscription(&o, someEvent));
}

TYPED_TEST(BasicsPatternsObserver, EventAvailableAttachOk) {
    using Event = TypeParam;
	InheretingObserver<Event> o;
	InheretingPublisher<Event> p;

	Event someEvent {};
	
	p.addEvent(someEvent);
	p.Attach(&o, niceValue, someEvent);
	
	ASSERT_TRUE(p.hasSubscription(&o, someEvent));
}

TYPED_TEST(BasicsPatternsObserver, UpdateWithNewValueNotOk_NoEvent) {
    using Event = TypeParam;
	InheretingObserver<Event> o;
	InheretingPublisher<Event> p;

	Event someEvent {};

	p.Attach(&o, niceValue, someEvent);

	ASSERT_FALSE(p.hasSubscription(&o, someEvent));
	
	p.pushUpdate(someEvent, 12.0);
	ASSERT_FALSE(test_global_values::testValue == 12.0);
	test_global_values::testValue = 0.0;
}

TYPED_TEST(BasicsPatternsObserver, UpdateWithNewValueOk) {
    using Event = TypeParam;
	InheretingObserver<Event> o;
	InheretingPublisher<Event> p;

	Event someEvent {};

    p.addEvent(someEvent);
	p.Attach(&o, niceValue, someEvent);

	ASSERT_TRUE(p.hasSubscription(&o, someEvent));

	p.pushUpdate(someEvent, 12.0);
	ASSERT_TRUE(test_global_values::testValue == 12.0);
	test_global_values::testValue = 0.0;
}

TYPED_TEST(BasicsPatternsObserver, UnableToAddRepetively) {
    using Event = TypeParam;
	InheretingObserver<Event> o;
	InheretingPublisher<Event> p;

	Event someEvent {};

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
    using Event = TypeParam;
	InheretingObserver<Event> o1, o2;
	InheretingPublisher<Event> p;

	Event someEvent {};

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
    using Event = TypeParam;
	InheretingObserver<Event> o1, o2;
	InheretingPublisher<Event> p;

	Event someEvent {};

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
