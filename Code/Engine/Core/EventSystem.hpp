#pragma once
#include "Engine/Core/HashedCaseInsensitiveString.hpp"
#include <vector>
#include <map>
#include <mutex>

class NamedProperties;
struct EventSubscriptionBase;
class EventSystem;

typedef NamedProperties EventArgs;
typedef bool (*EventCallbackFunction) (EventArgs& args);
typedef std::vector<EventSubscriptionBase*> SubscriptionList;

extern EventSystem* g_theEventSystem;

struct EventSubscriptionBase
{
	friend class EventSystem;

protected:
	EventSubscriptionBase(unsigned int priority) : m_priority(priority) {};
	virtual bool CallFunction(EventArgs& args) = 0;
	EventSubscriptionBase() = default;
	virtual ~EventSubscriptionBase() = default;
	
public:
	unsigned int m_priority = 0;	//Higher priority means it's executed first
};

template<typename T>
struct MethodEventSubscription : public EventSubscriptionBase
{
	friend class EventSystem;
	typedef bool (T::* EventCallbackObjectMethod)(EventArgs&);

protected:
	MethodEventSubscription(T& object, EventCallbackObjectMethod methodCallbackFunction, unsigned int priority) : EventSubscriptionBase(priority), m_object(object), m_methodCallbackFunction(methodCallbackFunction) {};
	T& m_object;
	EventCallbackObjectMethod m_methodCallbackFunction = nullptr;

	virtual bool CallFunction(EventArgs& args) override { return (m_object.*m_methodCallbackFunction)(args); };
};

struct FunctionEventSubscription : public EventSubscriptionBase
{
	friend class EventSystem;

protected:
	FunctionEventSubscription(EventCallbackFunction callbackFunctionPtr, unsigned int priority) : EventSubscriptionBase(priority), m_callbackFunctionPtr(callbackFunctionPtr) {};
	EventCallbackFunction		m_callbackFunctionPtr = nullptr;

	virtual bool CallFunction(EventArgs& args) override { return m_callbackFunctionPtr(args); };
};

struct EventSystemConfig
{

};

class EventSystem
{
public:
	EventSystem(EventSystemConfig const& config);
	~EventSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void SubscribeEventCallbackFunction(HashedCaseInsensitiveString const& eventName, EventCallbackFunction functionPtr, unsigned int priority = 0);
	void UnsubscribeEventCallbackFunction(HashedCaseInsensitiveString const& eventName, EventCallbackFunction functionPtr);
	
	/*
	//Added during SD4 (My version)
	template<typename T, bool (T::*method)(EventArgs&)>
	void SubscribeEventCallbackObjectMethod(HashedCaseInsensitiveString const& eventName, T& object);
	*/

	// (Squirrel's version)
	template<typename T>
	void SubscribeEventCallbackObjectMethod(HashedCaseInsensitiveString const& eventName, T& object, bool (T::*method)(EventArgs&), unsigned int priority = 0);

	template<typename T>
	void UnsubscribeEventCallbackObjectMethod(HashedCaseInsensitiveString const& eventName, T& object, bool (T::* method)(EventArgs&));

	bool FireEvent(HashedCaseInsensitiveString const& eventName, EventArgs& args);
	bool FireEvent(HashedCaseInsensitiveString const& eventName);

	std::vector<HCIString> GetRegisteredCommandNames();

protected:
	void SortSubscriptionsBasedOnPriority(SubscriptionList& list);

protected:
	EventSystemConfig m_config;
	std::map<HashedCaseInsensitiveString, SubscriptionList> m_subscriptionListsByEventName;
	std::recursive_mutex m_mutex;
};

/*
template<typename T, bool (T::*method)(EventArgs&) >
void EventSystem::SubscribeEventCallbackObjectMethod(HashedCaseInsensitiveString const& eventName, T& object)
{
	m_mutex.lock();

	MethodEventSubscription<T>* newSubscription = new MethodEventSubscription<T>(object, method);
	m_subscriptionListsByEventName[eventName].push_back(newSubscription);

	m_mutex.unlock();
}
*/

template<typename T>
void EventSystem::SubscribeEventCallbackObjectMethod(HashedCaseInsensitiveString const& eventName, T& object, bool (T::* method)(EventArgs&), unsigned int priority)
{
	m_mutex.lock();

	SubscriptionList& list = m_subscriptionListsByEventName[eventName];
	for (EventSubscriptionBase* subscription : list) {
		MethodEventSubscription<T>* methodSubscription = dynamic_cast<MethodEventSubscription<T>*>(subscription);
		if (methodSubscription && (&methodSubscription->m_object == &object) && (methodSubscription->m_methodCallbackFunction == method)) {
			m_mutex.unlock();
			return;
		}
	}

	MethodEventSubscription<T>* newSubscription = new MethodEventSubscription<T>(object, method, priority);
	list.push_back(newSubscription);
	SortSubscriptionsBasedOnPriority(list);
	m_mutex.unlock();
}

template<typename T>
inline void EventSystem::UnsubscribeEventCallbackObjectMethod(HashedCaseInsensitiveString const& eventName, T& object, bool(T::* method)(EventArgs&))
{
	m_mutex.lock();

	auto& list = m_subscriptionListsByEventName[eventName];
	for (auto iter = list.begin(); iter != list.end(); ++iter) {
		MethodEventSubscription<T>* methodSubscription = dynamic_cast<MethodEventSubscription<T>*>(*iter);
		// Check if the current subscription matches the object and method to be unsubscribed
		if (methodSubscription && (&methodSubscription->m_object == &object) && (methodSubscription->m_methodCallbackFunction == method)) {
			delete* iter; // Clean up the allocated memory
			list.erase(iter); // Remove the subscription from the list
			m_mutex.unlock();
			return; // Assuming only one subscription matches, we can return immediately after removing it
		}
	}

	m_mutex.unlock();
}
