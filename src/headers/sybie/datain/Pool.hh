//Pool.hh

#ifndef INCLUDE_SYBIE_DATAIN_POOL_HH
#define INCLUDE_SYBIE_DATAIN_POOL_HH

namespace sybie {
namespace datain {

class Pool
{
public:
    static Pool& GetGlobalPool();
public:
    Pool();
    ~Pool();
    Pool(const Pool& another) = delete;
    Pool(Pool&& another) = delete;
    Pool& operator=(const Pool& another) = delete;
    Pool& operator=(Pool&& another) = delete;

    void Set(const char* data_id, const char* data);
    const char* Get(const char* data_id) const;
private:
    void* _impl;
}; //class Pool

} //namespace datain
} //namespace sybie

#endif //ifndef
