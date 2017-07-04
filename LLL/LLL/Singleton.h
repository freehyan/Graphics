#pragma once
#include <boost/thread/mutex.hpp>

namespace GraphicsCommon
{
	template <class T>
	class CSingleton
	{
	public:
		static T* getInstance()
		{
			if (!m_pInstance)
			{
				boost::unique_lock<boost::mutex> Lock(m_Mutex);
				if (!m_pInstance)   //double checked locking
				{
					m_pInstance = new T;
					atexit(destroy);
				}
			}
			return m_pInstance;
		}

		static void destroy()
		{
			if (m_pInstance)
			{
				boost::unique_lock<boost::mutex> DestroyLock(m_Mutex);
				if (m_pInstance)
				{
					delete m_pInstance;
					m_pInstance = nullptr;  //this line is important when getInstance() is called after destroy()
				}
			}
		}

	protected:
		CSingleton(void) {}
		virtual ~CSingleton(void) {}

	private:
		static T* volatile  m_pInstance;
		static boost::mutex m_Mutex;
	};

	template <class T>
	T* volatile CSingleton<T>::m_pInstance = nullptr;

	template<class T>
	boost::mutex CSingleton<T>::m_Mutex;
}
