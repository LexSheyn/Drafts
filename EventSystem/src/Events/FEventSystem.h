#pragma once

#include <unordered_map>
#include <vector>
#include <functional>

namespace t3d
{
	enum class EPropagation
	{
		  Continue
		, Stop
	};

	enum class EEvent
	{
		  SomethingRequest
		, SomethingResponse
	};

	struct FDataOne {};
	struct FDataTwo {};

	struct FEventData
	{
		union
		{
			FDataOne One;
			FDataTwo Two;
		};
	};

	typedef bool bool8;

	using Callback_T = std::function<EPropagation(FEventData EventData)>;

	template<class C>
	using MemberCallback_T = EPropagation(C::*)(FEventData EventData);

	using StaticCallback_T = EPropagation(*)(FEventData EventData);

	struct FEvent
	{
		EEvent     Type;
		FEventData Data;
	};

	class FEventSystem
	{
	public:

		 FEventSystem () = default;
		~FEventSystem () = default;

	// Functions:

		void PollEvents ();

		void PushEvent (EEvent Event, FEventData EventData);

		template<class C>
		void Subscribe(EEvent Event, C* Instance, MemberCallback_T<C> Callback)
		{
			Callbacks[Event].push_back(Callback_T);
		}

		void Subscribe (EEvent Event, StaticCallback_T Callback);

		template<class C>
		void Unsubscribe (EEvent Event, MemberCallback_T<C> Callback);

		void Unsubscribe (EEvent Event, StaticCallback_T Callback);

	private:

	//	void Dispatch();
		std::vector<FEvent> WriteQueue;
		std::vector<FEvent> ReadQueue;

		std::unordered_map<EEvent, std::vector<Callback_T>> Callbacks;
	};



} // namespace t3d