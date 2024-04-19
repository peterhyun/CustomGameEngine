#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

EventSystem* g_theEventSystem = nullptr;

EventSystem::EventSystem(EventSystemConfig const& config):m_config(config)
{
}

EventSystem::~EventSystem()
{
}

void EventSystem::Startup()
{
}

void EventSystem::Shutdown()
{
}

void EventSystem::BeginFrame()
{
}

void EventSystem::EndFrame()
{
}

void EventSystem::SubscribeEventCallbackFunction(HCIString const& eventName, EventCallbackFunction functionPtr, unsigned int priority)
{
	m_mutex.lock();
	//Find if SubscriptionList exists from the eventName
	SubscriptionList& subscriptionList = m_subscriptionListsByEventName[eventName];
	
	for (auto registeredEventSubscriptionBase : subscriptionList) {
		FunctionEventSubscription* functionEventSubscriber = dynamic_cast<FunctionEventSubscription*>(registeredEventSubscriptionBase);
		if (functionEventSubscriber && functionEventSubscriber->m_callbackFunctionPtr == functionPtr) {
			m_mutex.unlock();
			return;
		}
	}

	subscriptionList.push_back(new FunctionEventSubscription(functionPtr, priority));
	SortSubscriptionsBasedOnPriority(subscriptionList);
	m_mutex.unlock();
}

void EventSystem::UnsubscribeEventCallbackFunction(HCIString const& eventName, EventCallbackFunction functionPtr)
{
	m_mutex.lock();
	//Get the subscription list if exists
	auto foundPair = m_subscriptionListsByEventName.find(eventName);
	if (foundPair != m_subscriptionListsByEventName.end()) {
		SubscriptionList& subscriptionList = foundPair->second;
		//If no subscribers, just delete the map entry
		if (subscriptionList.size() == 0) {
			m_subscriptionListsByEventName.erase(foundPair);
		}
		else {
			subscriptionList.erase(
				std::remove_if(subscriptionList.begin(), subscriptionList.end(), [functionPtr](EventSubscriptionBase* subscription) {
					FunctionEventSubscription* functionEventSubscriber = dynamic_cast<FunctionEventSubscription*>(subscription);
					if (functionEventSubscriber && functionEventSubscriber->m_callbackFunctionPtr == functionPtr) {
						delete functionEventSubscriber;
						return true;
					}
					return false;
				}), subscriptionList.end()
			);
		}
	}
	m_mutex.unlock();
}

bool EventSystem::FireEvent(HCIString const& eventName, EventArgs& args)
{
	m_mutex.lock();
	auto foundPair = m_subscriptionListsByEventName.find(eventName);
	if (foundPair != m_subscriptionListsByEventName.end()) {
		const SubscriptionList& subscriptionList = foundPair->second;
		for (int i = 0; i < (int)subscriptionList.size() ; i++) {
			bool isEventConsumed = subscriptionList[i]->CallFunction(args);
			if(isEventConsumed) {
				m_mutex.unlock();
				return true;
			}
		}
		m_mutex.unlock();
		return false;
	}
	else {
		if (g_theDevConsole == nullptr) {
			ERROR_RECOVERABLE(Stringf("Trying to send Unknown Command(%s) error to DevConsole but g_theDevConsole is not initialized.", eventName.c_str()));
		}
		g_theDevConsole->AddLine(DevConsole::ERROR, std::string(eventName) + " is an unknown command");
		m_mutex.unlock();
		return false;
	}
}

bool EventSystem::FireEvent(HCIString const& eventName)
{
	m_mutex.lock();
	auto foundPair = m_subscriptionListsByEventName.find(eventName);
	if (foundPair != m_subscriptionListsByEventName.end()) {
		const SubscriptionList& subscriptionList = foundPair->second;
		for (int i = 0; i < (int)subscriptionList.size(); i++) {
			EventArgs emptyArgs;
			bool isEventConsumed = subscriptionList[i]->CallFunction(emptyArgs);
			if (isEventConsumed) {
				m_mutex.unlock();
				return true;
			}
		}
		m_mutex.unlock();
		return false;
	}
	else {
		if (g_theDevConsole == nullptr) {
			ERROR_RECOVERABLE("Trying to send Unknown Command error to DevConsole but g_theDevConsole is not initialized.");
		}
		g_theDevConsole->AddLine(DevConsole::ERROR, std::string(eventName) + " is an unknown command");
		m_mutex.unlock();
		return false;
	}
}

std::vector<HCIString> EventSystem::GetRegisteredCommandNames()
{
	m_mutex.lock();
	std::vector<HCIString> registeredCommandNames;
	for (auto const& subscriptionListIterator : m_subscriptionListsByEventName) {
		registeredCommandNames.push_back(subscriptionListIterator.first);
	}
	m_mutex.unlock();
	return registeredCommandNames;
}

void EventSystem::SortSubscriptionsBasedOnPriority(SubscriptionList& list)
{
	// Sort the subscriptions for the current event
	std::sort(list.begin(), list.end(),
		[](EventSubscriptionBase* a, EventSubscriptionBase* b) {
			// Sort by priority, higher priority comes first
			return a->m_priority > b->m_priority;
		});
}
