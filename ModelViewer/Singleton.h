#pragma once

namespace ModelViewer
{
	template<class T>
	class Singleton
	{
	public:
		static T& Instance()
		{
			static T instance;
			return instance;
		}
		Singleton(T const&) = delete;
		void operator=(T const&) = delete;

	protected:
		Singleton() {}
		~Singleton() {}
	};
}