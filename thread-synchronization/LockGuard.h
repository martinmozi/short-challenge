namespace Challenge
{
    template<typename _Mutex>
    class lock_guard
    {
        _Mutex & m_mutex;
    public:
        explicit lock_guard(std::mutex & mutex)
        :   m_mutex(mutex)
        {
            m_mutex.lock();
        }
        
        ~lock_guard()
        {
            m_mutex.unlock();
        }
        
        lock_guard(const lock_guard&) = delete;
        lock_guard& operator=(const lock_guard&) = delete;
    };
}
