#pragma once
#include "tool.h"
#include <atomic>
namespace PO::Tool
{
	struct intrustive_ptr_ref_wrapper
	{
		template<typename type>
		static void add_ref(type* t) noexcept { t->add_ref(); }
		template<typename type>
		static void sub_ref(type* t) noexcept { t->sub_ref(); }
	};

	struct intrusive_object_base
	{
		void add_ref() const noexcept { m_ref.add_ref(); }
		void sub_ref() const noexcept {
			if (m_ref.sub_ref()) {
				intrusive_object_base* ptr = const_cast<intrusive_object_base*>(this);
				ptr->release();
			}
		}
	protected:
		virtual void release() noexcept = 0;
	private:
		mutable Tool::atomic_reference_count m_ref;
	};

	template<typename T>
	struct intrusive_object
	{
		void add_ref() const noexcept { m_ref.add_ref(); }
		void sub_ref() const noexcept {
			if (m_ref.sub_ref()) {
				const_cast<T&>(static_cast<const T&>(*this)).release();
			}
		}
	private:
		mutable Tool::atomic_reference_count m_ref;
	};

	template<typename T>
	struct intrusive_standard_object
	{
		void add_ref() const noexcept { m_ref.add_ref(); }
		void sub_ref() const noexcept {
			if (m_ref.sub_ref()) {
				delete const_cast<T*>(static_cast<const T*>(this));
			}
		}
	private:
		mutable Tool::atomic_reference_count m_ref;
	};

	template<typename type, typename wrapper = intrustive_ptr_ref_wrapper>
	struct intrusive_ptr
	{
		void reset() noexcept { if (m_ptr != nullptr) { wrapper::sub_ref(m_ptr); m_ptr = nullptr; } }

		intrusive_ptr(type* t) noexcept : m_ptr(t) { if (t != nullptr) wrapper::add_ref(m_ptr); }
		template<typename = std::void_t<std::enable_if_t<!std::is_const_v<type>>>>
		intrusive_ptr(const type* t) noexcept : m_ptr(t) { if (t != nullptr) wrapper::add_ref(m_ptr); }
		intrusive_ptr() noexcept : m_ptr(nullptr) {}
		template<typename o_type, typename = std::enable_if_t<std::is_base_of_v<type, o_type>>>
		intrusive_ptr(o_type* t) noexcept : intrusive_ptr(static_cast<type*>(t)) {}
		template<typename o_type, typename = std::enable_if_t<std::is_base_of_v<type, o_type>>>
		intrusive_ptr(const o_type* t) noexcept : intrusive_ptr(static_cast<const type*>(t)) {}

		intrusive_ptr(intrusive_ptr&& ip) noexcept : m_ptr(ip.m_ptr) { ip.m_ptr = nullptr; }
		intrusive_ptr(const intrusive_ptr& ip) noexcept : intrusive_ptr(ip.m_ptr) {}

		intrusive_ptr& operator=(intrusive_ptr&& ip) noexcept {
			intrusive_ptr tem(std::move(ip));
			reset();
			m_ptr = tem.m_ptr;
			tem.m_ptr = nullptr;
			return *this;
		}

		intrusive_ptr& operator=(const intrusive_ptr& ip) noexcept {
			intrusive_ptr tem(ip);
			return this->operator=(std::move(tem));
		}
		template<typename o_type, typename = std::enable_if_t<std::is_base_of_v<type, o_type>>>
		intrusive_ptr(intrusive_ptr<o_type> ip) : m_ptr(ip.m_ptr) { ip.m_ptr = nullptr; }

		~intrusive_ptr() noexcept { reset();}

		bool operator== (const intrusive_ptr& ip) const noexcept { return m_ptr == ip.m_ptr; }
		bool operator!= (const intrusive_ptr& ip) const noexcept { return m_ptr != ip.m_ptr; }
		bool operator<(const intrusive_ptr& ip) const noexcept { return m_ptr < ip.m_ptr; }
		bool operator<=(const intrusive_ptr& ip) const noexcept { return m_ptr <= ip.m_ptr; }
		operator type*() noexcept { return m_ptr; }
		operator const type*() const noexcept { return m_ptr; }
		type& operator*() noexcept { return *m_ptr; }
		const type& operator*() const noexcept { return *m_ptr; }
		type* operator->() noexcept { return m_ptr; }
		const type* operator->() const noexcept { return m_ptr; }
		operator bool() const noexcept { return m_ptr != nullptr; }
		template<typename o_type, typename o_wrapper = intrustive_ptr_ref_wrapper>
		intrusive_ptr<o_type, o_wrapper> cast_static() noexcept
		{
			return intrusive_ptr<o_type, o_wrapper>{static_cast<o_type*>(m_ptr)};
		}
		/*
		template<typename o_type, typename o_wrapper = intrustive_ptr_ref_wrapper> 
		std::enable_if_t<std::is_base_of_v<type, o_type> || std::is_base_of_v<o_type, type>, intrusive_ptr<o_type, o_wrapper>>
			cast_static() noexcept
		{
			return intrusive_ptr<o_type, o_wrapper>{static_cast<o_type*>(this)};
		}
		*/
	private:
		template<typename o_type, typename wrapper> friend struct intrusive_ptr;
		type * m_ptr;
	};

	struct intrustive_ptr_strong_ref_wrapper
	{
		template<typename type>
		static void add_ref(type* t) noexcept { t->strong_add_ref(); }
		template<typename type>
		static void sub_ref(type* t) noexcept { t->strong_sub_ref(); }
		template<typename type>
		static size_t count(type* t) noexcept { t->count(); }
	};

	namespace Implement
	{
		template<typename T> struct as_const;
		template<typename T, typename wrapper> struct as_const<Tool::intrusive_ptr<T, wrapper>> {
			using type = Tool::intrusive_ptr<const T, wrapper>;
		};
	}

	template<typename T> using as_const = typename Implement::as_const<T>::type;

	/*
	template<typename T, typename strong_weapper = intrustive_ptr_strong_ref_wrapper, typename weak_wrapper = intrustive_ptr_ref_wrapper>
	struct intrusive_strong_ptr
	{
		intrusive_strong_ptr(intrusive_strong_ptr&& isp)  = default;
		intrusive_strong_ptr(const intrusive_strong_ptr& isp) : m_ptr(isp.m_ptr) { if (m_ptr) strong_weapper::add_ref(m_ptr); }
		template<typename o_T>
		intrusive_strong_ptr(intrusive_strong_ptr<o_T, strong_weapper, weak_wrapper> isp) 
		{
			m_ptr = isp.m_ptr;
			if(m_ptr)
				strong_weapper::add_ref(m_ptr);
		}
		~intrusive_strong_ptr() {
			reset();
		}
		void reset() {
			if(m_ptr)
			{
				strong_weapper::sub_ref<T>(m_ptr);   
				m_ptr.reset();
			}
		}
	private:
		intrusive_ptr<T, weak_wrapper> m_ptr;
	};

	template<typename T, typename strong_weapper = intrustive_ptr_strong_ref_wrapper, typename weak_wrapper = intrustive_ptr_ref_wrapper>
	struct intrusive_weak_ptr
	{
		intrusive_weak_ptr(intrusive_strong_ptr<T, strong_weapper, weak_wrapper> isp) : m_ptr(std::move(isp.m_ptr)){}
		operator bool () const noexcept{return m_ptr && strong_weapper::count<T>(m_ptr) != 0;}
		void reset() noexcept {m_ptr.reset();}
	private:
		intrusive_ptr<T, weak_wrapper> m_ptr;
	};

	
	struct intrusice_strong_bject_base : intrusive_object_base
	{
		void strong_add_ref() noexcept { m_ref.add_ref(); }
		void strong_sub_ref() noexcept { m_ref.sub_ref(); }
		size_t count() const noexcept {return m_ref.count(); }
	private:
		Tool::atomic_reference_count m_ref;
	};
	*/







	
	/*
	template<typename type, typename deleter_type = std::default_delete<type>>
	class intrusive_ptr
	{
		type* data;
		deleter_type del;
	public:
		const deleter_type& deleter() const { return del; }
		bool operator== (const intrusive_ptr& ip) const noexcept { return data == ip.data; }
		bool operator!= (const intrusive_ptr& ip) const noexcept { return data != ip.data; }
		operator bool() const noexcept { return data != nullptr; }

		intrusive_ptr(type* t) : data(t) { if (data != nullptr) data->add_ref(); }

		intrusive_ptr(type* t, const deleter_type& dt) : data(t), del(dt) { if (data != nullptr) { data->add_ref(); } }

		template<typename delete_t>
		intrusive_ptr(type* t, delete_t&& dt) : data(t), del(std::forward<delete_t>(dt)) { if (data != nullptr) data->add_ref(); }

		intrusive_ptr() noexcept : data(nullptr) {}

		intrusive_ptr(intrusive_ptr&& ip) noexcept : data(ip.data), del(std::move(ip.del)) { ip.data = nullptr; }
		intrusive_ptr(const intrusive_ptr& ip) noexcept : data(ip.data), del(ip.del) { if (data != nullptr) data->add_ref(); }

		
		void reset() noexcept
		{
			if (data != nullptr && data->sub_ref())
				del(data);
			data = nullptr;
		}

		~intrusive_ptr() {
			if (data != nullptr && data->sub_ref())
				del(data);
		}

		intrusive_ptr& operator=(intrusive_ptr ip)  noexcept
		{
			intrusive_ptr tem(std::move(ip));
			intrusive_ptr::reset();
			data = tem.data;
			del = std::move(tem.del);
			tem.data = nullptr;
			return *this;
		}

		operator type* () noexcept { return data; }
		operator const type* () const noexcept { return data; }
		type* operator->() noexcept { return data; }
		const type* operator->() const noexcept { return data; }
		type& operator* () noexcept { return *data; }
		const type& operator*() const noexcept { return *data; }
		template<typename other_type> operator intrusive_ptr<other_type>() const noexcept { return intrusive_ptr<other_type>{data }; }
		type* ptr() noexcept { return data; }
		const type* ptr() const noexcept { return data; }
	};
	*/
	/*
	template<typename hold_t, typename deleter_t = std::default_delete<hold_t>>
	struct intrusive_owner_ptr
	{
		intrusive_owner_ptr() = default;
		intrusive_owner_ptr(hold_t* p) : ptr(p) { if (ptr) ptr->add_owner_ref(); }
		intrusive_owner_ptr(hold_t* p, deleter_t dt) : ptr(p, std::move(dt)) { if (ptr) ptr->add_owner_ref(); }
		template<typename deleter_tt>
		intrusive_owner_ptr(hold_t* p, deleter_tt&& dt) : ptr(p, std::forward<deleter_tt>(dt)) { if (ptr) ptr->add_owner_ref(); }
		intrusive_owner_ptr(const intrusive_owner_ptr& iop) : ptr(iop.ptr) { if (ptr) ptr->add_owner_ref(); }
		intrusive_owner_ptr(intrusive_owner_ptr&& iop) : ptr(std::move(iop.ptr)) {}
		template<typename o_hold_t, typename o_deleter_t>
		intrusive_owner_ptr(const intrusive_owner_ptr<o_hold_t, o_deleter_t>& iop) : ptr(iop.ptr) { if (ptr) ptr->add_owner_ref(); }
		template<typename o_hold_t, typename o_deleter_t>
		intrusive_owner_ptr(intrusive_owner_ptr<o_hold_t, o_deleter_t>&& iop) : ptr(std::move(iop.ptr)) {}
		//intrusive_owner_ptr& operator=(intrusive_owner_ptr iop) { ptr = std::move(iop.ptr); return *this; }
		intrusive_owner_ptr& operator=(intrusive_owner_ptr&& iop) { 
			intrusive_owner_ptr tem(std::move(iop));
			ptr = std::move(tem.ptr);
			return *this; 
		}
		~intrusive_owner_ptr() { if (ptr) ptr->sub_owner_ref(); }

		hold_t* operator->() noexcept { return ptr; }
		const hold_t* operator->() const noexcept { return ptr; }
		hold_t& operator*() noexcept { return *ptr; }
		const hold_t& operator*() const noexcept { return *ptr; }
		operator hold_t*() noexcept { return ptr; }
		operator const hold_t*() const noexcept { return ptr; }
		operator bool() const noexcept { return ptr; }

		template<typename hold_t, typename deleter_t> friend struct intrusive_owner_ptr;
		template<typename hold_t, typename deleter_t> friend struct intrusive_viewer_ptr;

	private:

		intrusive_ptr<hold_t, deleter_t> ptr;
	};

	template<typename hold_t, typename deleter_t = std::default_delete<hold_t>>
	struct intrusive_viewer_ptr : intrusive_ptr<hold_t, deleter_t>
	{
		intrusive_viewer_ptr(intrusive_owner_ptr<hold_t, deleter_t> iop) : ptr(std::move(iop.ptr)) {}
		intrusive_viewer_ptr& operator=(intrusive_viewer_ptr ivp) { ptr = std::move(iop.ptr); return *this; }
		operator bool() const noexcept { return ptr && ptr->owner_count.count() != 0; }
	private:
		intrusive_ptr<hold_t, deleter_t> ptr;
	};
	*/
}