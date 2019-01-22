#pragma once

#include "Configuration.h"

#include <vector>
#include <memory>
#include <atomic>

#define NO_COPY(classname) \
    classname(const classname &) = delete; \
classname &operator=(const classname &) = delete

#define NO_MOVE(classname) \
    classname(classname &&) = delete; \
classname &operator=(classname &&) = delete


#define DEFAULT_COPY(classname) \
    classname(const classname &) = default; \
classname &operator=(const classname &) = default

#define DEFAULT_MOVE(classname) \
    classname(classname &&) = default; \
classname &operator=(classname &&) = default

#define CUSTOM_COPY(classname)  inline classname(const classname &other) \
{ \
    *this = other; \
} \
classname &operator=(const classname &other)

#define CUSTOM_MOVE(classname)  inline classname(classname &&other) \
{ \
    *this = std::move(other); \
} \
classname &operator=(classname &&other)

class ICancellationToken
{
    public:
        virtual ~ICancellationToken();

        virtual bool IsCancelled() const = 0;
};

class CancellationToken : public ICancellationToken
{
    public:
        CancellationToken();
        virtual ~CancellationToken();

        NO_COPY(CancellationToken);
        NO_MOVE(CancellationToken);

        void Cancel();
        bool IsCancelled() const override;
        void Reset();

    private:
        std::atomic<bool> m_Cancelled;
};

inline ICancellationToken::~ICancellationToken() { }

inline CancellationToken::CancellationToken() : m_Cancelled(false) { }

inline CancellationToken::~CancellationToken() {}

inline void CancellationToken::Cancel()
{
    m_Cancelled.store(true);
}

inline bool CancellationToken::IsCancelled() const
{
    return m_Cancelled.load();
}

inline void CancellationToken::Reset()
{
    m_Cancelled.store(false);
}
