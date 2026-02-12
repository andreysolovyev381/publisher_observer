//
// Created by Andrey Solovyev on 19.12.2022.
//

#pragma once

#include "requirements/hash.h"
#include "requirements/comparator.h"
#include "requirements/ctor_input.h"
#include "requirements/container.h"

#include <boost/circular_buffer.hpp>

#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace culib::patterns {
	
	/**
	 * @dev
	 * Gang of Four book defined Publisher Observer pattern as a lazy one,
	 * where a Publisher just notifies an Observer, that there is a new state,
	 * and it is up to an Observer to decide if it is willing to update itself with that
	 * new state of a Publisher.
	 * However, in this case we have a situation that every Event MUST be processed
	 * by an Observer, therefore we use push model, to assure that happens.
	 * Indeed, Publisher can't wait until Observer finishes, so there must be a Queue to handle
	 * incoming data.
	 * Such a Queue can be turned into thread safe, although at the moment of coding this it is hardy
	 * imaginable that such Queue can be an object of concurrent access.
	 *
	 * */

	template<typename Event, typename Value>
	struct Observer {

		using event_type = Event;
		using value_type = Value;
		using observer_type = Observer<Event, Value>;

		struct EventValues {
			using Data = std::vector<std::pair<Event, boost::circular_buffer<Value>>>;
			using Iter = typename Data::iterator;
			using CIter = typename Data::const_iterator;
			
			Data data;
			
			auto find(event_type const& event) noexcept {
				auto found {std::find_if(data.begin(), data.end(), [&event](auto const& p){
					return event == p.first;
				})};
				return found;
			}
			
			auto begin() noexcept { return data.begin(); }
			auto begin() const noexcept { return data.cbegin(); }
			auto cbegin() const noexcept { return data.cbegin(); }
			auto end() noexcept { return data.end(); }
			auto end() const noexcept { return data.cend(); }
			auto cend() const noexcept { return data.cend(); }
			
			auto const& front() const noexcept { return data.front(); }
            auto& front() noexcept { return data.front(); }
            auto const& back() const noexcept { return data.back(); }
            auto& back() noexcept { return data.back(); }

			std::pair<Iter, bool> emplace(Event event, boost::circular_buffer<Value> cb) {
				auto foundEvent {find(event)};
				if (foundEvent == end()) {
					data.emplace_back(std::move(event), std::move(cb));
					return std::pair{std::prev(data.end()), true};
				}
				return std::pair{foundEvent, false};
			}

			std::pair<Iter, bool> emplace(std::pair<Event, boost::circular_buffer<Value>> p) {
				auto foundEvent {find(p.first)};
				if (foundEvent == end()) {
					data.psuh_back(std::move(p));
					return std::pair{std::prev(data.end()), true};
				}
				return std::pair{foundEvent, false};
			}
			
			void erase(event_type const& event) {
				auto foundEvent {find(event)};
				if (foundEvent == end()) {
					return;
				}
				std::iter_swap(foundEvent, std::prev(data.end()));
				data.pop_back();
			}
			
			std::size_t size() const noexcept {
				return data.size();
			}
			
			bool empty() const noexcept {
				return data.empty();
			}
		};
		
		Observer() = default;

		virtual ~Observer() = default;

		void bookEvent(Event const& event) {
			eventValues.emplace(event, boost::circular_buffer<Value>(eventsLength));
		}

		void removeEvent(Event const& event) {
			eventValues.erase(event);
		}

		virtual void updateCallback([[maybe_unused]] Event const& event, [[maybe_unused]] Value const& value) & {
//todo uncomment and update after MT is done
#if 0
			auto found = eventValues.find(event);
			if (found == eventValues.end()) {
				return;
			}
			auto& [_, values] = *found;
			if (values.size() < eventsLength) {
				values.resize(eventsLength);
			}
			values.push_back(value);
#endif
		}

		EventValues eventValues;
		std::size_t eventsLength {1u};
	};

	template<typename Event, typename Value, typename Hash = std::hash<Event>, typename Equal = std::equal_to<Event>>
	requires ::culib::requirements::IsHash<Event, Hash> && ::culib::requirements::IsComparator<Event, Equal>
	class Publisher {
	public:

		using event_type = Event;
		using value_type = Value;
		using hash_type = Hash;
		using equality_type = Equal;
		using publisher_type = Publisher<Event, Value, Hash, Equal>;

		using ObserverType = Observer<event_type, value_type>;

		Publisher() = default;
		virtual ~Publisher() = default;

		template<typename... Events>
		void Attach(ObserverType *observer, int niceValue, Events const&... events) &
		requires ::culib::requirements::AllTheSame<Event, Events...>
		{
			(AttachImpl(observer, niceValue, events), ...);
		}

		template<typename... Events>
		void Detach(ObserverType *observer, Events const&... events) &
		requires ::culib::requirements::AllTheSame<Event, Events...>
		{
			(DetachImpl(observer, events), ...);
		}

		template<::culib::requirements::IsContainer Container>
		void Attach(ObserverType *observer, int niceValue, Container const& events) &
		requires std::same_as<typename Container::value_type, Event>
		{
			for (auto const& event : events) {
				AttachImpl(observer, niceValue, events);
			}
		}

		template<::culib::requirements::IsContainer Container>
		void Detach(ObserverType *observer, Container const& events) &
		requires std::same_as<typename Container::value_type, Event>
		{
			for (auto const& event : events) {
				DetachImpl(observer, events);
			}
		}

		void pushUpdate(Event const& event, Value const& newValue) const & {
			std::vector<std::pair<int, ObserverType*>> const& relevantObservers { getObservers(event) };
			//todo optimizable for work contracts
			for (auto [niceValue, observerPtr] : relevantObservers) {
				observerPtr->updateCallback(event, newValue);
			}
		}

		void addEvent (Event const& event) & {
			if (!eventExists(event)) {
				events_.emplace(event, std::vector<std::pair<int, ObserverType*>>{});
			}
		}

		void removeEvent (Event const& event) & {
			events_.erase(event);
		}

		bool eventExists (Event const& event) const & noexcept {
			return events_.find(event) != events_.end();
		}

		bool hasSubscription(ObserverType *observer, Event const& event) const & noexcept {
			auto foundObserver = observers.find(observer);
			if (foundObserver == observers.end()) {
				return false;
			}
			auto foundSubscription = foundObserver->second.find(event);
			return foundSubscription != foundObserver->second.end();
		}

		std::vector<std::pair<int, ObserverType*>> const& getObservers(Event const& event) const & noexcept {
            auto foundObservers = events_.find(event);
            if (foundObservers == events_.end()) {
                return emptyObservers;
            }
            return foundObservers->second;
		}

	protected:
        //todo - optimizable, Event can be heavy (unlikely, but...)
        std::unordered_map<ObserverType*, std::unordered_set<Event, Hash, Equal>> observers;
        static constexpr inline std::vector<std::pair<int, ObserverType*>> emptyObservers {};
        std::unordered_map<Event, std::vector<std::pair<int, ObserverType*>>, Hash, Equal> events_;
    
    protected:
    
		void AttachImpl(ObserverType *observer, int niceValue, Event const& event) {
            auto foundEvent = events_.find(event);
            if (foundEvent == events_.end()) {
                //todo must be logged, no event
                return;
            }
            
            auto foundObserver = observers.find(observer);
            if (foundObserver != observers.end()) {
                if (auto alreadyBooked = foundObserver->second.find(event);
                    alreadyBooked != foundObserver->second.end())
                {
                    //todo must be logged, observer already booked for event
                    return;
                }
            }
			
			std::vector<std::pair<int, ObserverType*>> & relevantObservers {foundEvent->second};
            if (relevantObservers.empty()) {
                relevantObservers.reserve(4u); //arbitrary figure, expected observes quantity
                relevantObservers.emplace_back(niceValue, observer);
            }
            else {
                relevantObservers.emplace_back(niceValue, observer);
                auto iterNewElem {std::prev(relevantObservers.end(), 1)};
                //debug build check
                assert(relevantObservers.size() >= 2u);
                auto iterEvents {std::prev(relevantObservers.end(), 2)};
                while (iterEvents >= relevantObservers.begin() && iterEvents->first > niceValue) {
                    std::iter_swap(iterEvents, iterNewElem);
                    --iterNewElem;
                    --iterEvents;
                }
            }
            observers[observer].emplace(event);
			observer->bookEvent(event);
		}

		void DetachImpl(ObserverType *observer, Event const& event) {
            auto foundObservers = events_.find(event);
            if (foundObservers == events_.end()) {
                return;
            }
			std::vector<std::pair<int, ObserverType*>> & relevantObservers {foundObservers->second};
            auto found {std::find_if(relevantObservers.begin(), relevantObservers.end(), [observer](auto elem){
                return elem.second == observer;
            })};
            if (found != relevantObservers.end()) {
                relevantObservers.erase(found);
            }
			observers[observer].erase(event);
			observer->removeEvent(event);
		}
	};

	template<typename Publisher, typename Observer>
	static constexpr inline bool checkPublisherObserver () {
		static_assert(std::is_same_v<typename Publisher::event_type, typename Observer::event_type>, "Event types are different");
		static_assert(std::is_same_v<typename Publisher::value_type, typename Observer::value_type>, "Value types are different");

		return true;
	}
	
	
	namespace requirements {
		
		template<typename T>
		concept IsObserver = std::same_as<T, typename T::observer_type>;
		
		template<typename T>
		static constexpr inline bool is_observer_v { IsObserver<T> };
		
		
		template<typename T>
		concept IsPublisher = std::same_as<T, typename T::publisher_type>;
		
		template<typename T>
		static constexpr inline bool is_publisher_v { IsPublisher<T> };
		
	}//!namespace requirements
	
} //!namespace
