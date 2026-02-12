//
// Created by Andrey Solovyev on 25/12/2023.
//

#pragma once

#include <gtest/gtest.h>

#include "../include/observer.hpp"

#include <string>

namespace test_global_values {

	static inline double testValue{0.0};

}

using event_types = testing::Types<
		int,
		std::string
>;

template <typename Event>
struct InheretingObserver : public culib::patterns::Observer<Event, double> {

	Event testEvent;

	void updateCallback(Event const& event, double const& value) & override {
		testEvent = event;
		test_global_values::testValue = value;
	}
};

template <typename Event>
struct InheretingObserverFail : public culib::patterns::Observer<Event, double>
{};
