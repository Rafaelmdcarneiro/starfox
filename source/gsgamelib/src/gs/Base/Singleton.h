#ifndef _SINGLETON_H_
#define _SINGLETON_H_

// This template class can be used to easily create a singleton class.
// To use it, simply derive your class from "Singleton<YOUR_CLASS_NAME>"
// and make your class's constructor protected (not private).
//
// To get the single instance of your class, call the static Instance()
// through your class, ex: "YOUR_CLASS_NAME::Instance()"

template <class T>
class Singleton
{
public:
	// Returns true if the singleton instance has been created, false otherwise
	inline static bool IsInstantiated();

	// Retrieves the single instance (creates it if it doesn't exist)
	inline static T& Instance();

protected:		
	Singleton() {}
	~Singleton() {} // Protected so instance cannot be deleted from outside

private:
	// Declare a destroyer for the singleton instance
	template <class T>
	class InstanceDestroyer
	{
	public:
		InstanceDestroyer() : m_pInstance(nullptr) {}
		~InstanceDestroyer() { delete m_pInstance; }
		void SetInstance(T* pInstance) { m_pInstance = pInstance; }

	private:
		T* m_pInstance;
	};

private:
	static T* m_pInstance;

	// This destroyer instance will be destructed on program exit, deleting 
	// the singleton instance with it (if any)
	static InstanceDestroyer<T> m_destroyer;
};

template <class T>
T* Singleton<T>::m_pInstance = nullptr;

template <class T>
typename Singleton<T>::InstanceDestroyer<T> Singleton<T>::m_destroyer;

template <class T>
inline bool Singleton<T>::IsInstantiated()
{
	return (m_pInstance != nullptr);
}

template <class T>
inline T& Singleton<T>::Instance()
{
	if (!m_pInstance)
	{
		m_pInstance = new T;
		m_destroyer.SetInstance( m_pInstance );
	}

	return *m_pInstance;
}

#endif // _SINGLETON_H_
