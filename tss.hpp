#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <pthread.h>

namespace boost{
namespace prototype{

namespace detail{

struct tss_entry{
	tss_entry()
		: userptr(0),key(0){}
	tss_entry(void *userptr,unsigned int key,void (*dispose)(void *))
		: userptr(userptr),key(key),dispose(dispose){}
	
	void reset(){
		this->userptr=0;
		this->key=0;
	}

	void *userptr;
	unsigned int key;
	void (*dispose)(void *);
};

typedef std::vector<tss_entry> tss_vector_type;

void cleanup_tss(void *voidvec){
	BOOST_ASSERT(voidvec);
	tss_vector_type &vec=*static_cast<tss_vector_type *>(voidvec);
	for(tss_vector_type::iterator it=vec.begin();it != vec.end();++it){
		if(it->userptr) it->dispose(it->userptr);
	}
}

#ifdef __GNUG__

pthread_key_t key;
pthread_once_t key_once = PTHREAD_ONCE_INIT;
__thread tss_vector_type *tss_vector=0;

void make_key(){
	BOOST_VERIFY(pthread_key_create(&key,&cleanup_tss) == 0);
}

tss_vector_type &get_tss_vector(){
	tss_vector_type *ptr=tss_vector;
	if(!ptr){
		BOOST_VERIFY(pthread_once(&key_once,&make_key) == 0);
		tss_vector=ptr=new tss_vector_type;
		BOOST_VERIFY(pthread_setspecific(key,ptr) == 0);
	}
	return *ptr;
}

tss_vector_type *try_get_tss_vector(){
	return tss_vector;
}

#else
#error
#endif



struct tss_id{
	tss_id(unsigned int key,std::size_t index) : key(key),index(index){}
	unsigned int key;
	std::size_t index;
};

struct id_allocator{
public:
	id_allocator()
		: indexes_end(0)
		, keys_end(1){}
	tss_id allocate(){
		lock_guard<mutex_type> l(this->mutex);
		return tss_id(this->allocate_key(),this->allocate_index());
	}
	void free(tss_id const &id){
		lock_guard<mutex_type> l(this->mutex);
		this->free_indexes.push_back(id.index);
	}
private:
	std::size_t allocate_index(){
		if(this->free_indexes.empty()){
			return this->indexes_end++;
		}else{
			std::size_t tmp=this->free_indexes.back();
			this->free_indexes.pop_back();
			return tmp;
		}
	}
	unsigned int allocate_key(){
		return this->keys_end++;
	}

	typedef boost::mutex mutex_type;
	mutex_type mutex;
	std::vector<std::size_t> free_indexes;
	std::size_t indexes_end;
	unsigned int keys_end;
};

id_allocator allocator;

template<class T>
void disposer(void *ptr){
	delete static_cast<T *>(ptr);
}

}


template<class T>
class thread_specific_ptr{
public:
	thread_specific_ptr()
		: id(detail::allocator.allocate()){}
	~thread_specific_ptr(){
		try{
			detail::allocator.free(this->id);
		}catch(thread_resource_error &){}
	}
	void reset(T *new_value=0){
		using namespace detail;
		tss_vector_type &vec=get_tss_vector();
		if(this->id.index < vec.size()){
			tss_entry &entry=vec[this->id.index];
			//no need to compare key
			if(new_value != entry.userptr) entry.dispose(entry.userptr);
		}else{
			vec.resize(this->id.index + 1);
		}
		vec[this->id.index]=tss_entry(new_value,this->id.key,&disposer<T>);
	}
	T *get() const{
		using namespace detail;
		tss_vector_type *vec=try_get_tss_vector();
		if(vec && this->id.index < vec->size()){
			tss_entry &entry=(*vec)[this->id.index];
			if(entry.key == this->id.key) return static_cast<T *>(entry.userptr);
		}
		return 0;
	}
	T *operator->() const{
		using namespace detail;
		tss_vector_type *vec=try_get_tss_vector();
		BOOST_ASSERT(vec);
		BOOST_ASSERT(this->id.index < vec->size());
		tss_entry &entry=(*vec)[this->id.index];
		BOOST_ASSERT(entry.key == this->id.key);
		BOOST_ASSERT(entry.userptr);
		return static_cast<T *>(entry.userptr);
	}
	T &operator*() const{
		return *this->operator->();
	}
	T *release(){
		using namespace detail;
		tss_vector_type *vec=try_get_tss_vector();
		if(vec && this->id.index < vec->size()){
			tss_entry &entry=(*vec)[this->id.index];
			if(entry.key == this->id.key){
				T *tmp=static_cast<T *>(entry.userptr);
				entry.reset();
				return tmp;
			}
		}	
		return 0;	
	}
private:
	detail::tss_id id;
};

}
}