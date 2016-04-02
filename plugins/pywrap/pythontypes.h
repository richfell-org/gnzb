/*
	gnzb python common - gnzb python integration

    Copyright (C) 2016  Richard J. Fellinger, Jr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, see <http://www.gnu.org/licenses/> or write
	to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301 USA.
*/
#ifndef __PYTHON_TYPES_HEADER__
#define __PYTHON_TYPES_HEADER__

#include <Python.h>
#include <memory>
#include <stdexcept>
#include <cstdarg>

#if defined(DEBUG) || defined(DEBUG_PYREF)
#   include<iostream>
#endif

#include "pythonutil.h"

namespace PyWrap {

inline PyObject *INCREF(PyObject *p_obj) { if(p_obj != nullptr) Py_INCREF(p_obj); return p_obj; }

/**
 * Py_DECREF wrapped as a deleter
 */
struct PythonDecref
{
#ifdef DEBUG_PYREF
	void operator ()(PyObject* p_pyobj);
#else
	void operator ()(PyObject* p_pyobj) { if(Py_REFCNT(p_pyobj) > 0) Py_DECREF(p_pyobj); }
#endif  /* DEBUG_PYREF */
};

using PyObjectPtr = std::unique_ptr<PyObject, PythonDecref>;

class Object;
#if PY_MAJOR_VERSION >= 3
	class Unicode;
#	define PYSTRING	Unicode
#else
	class Int;
	class String;
#	define PYSTRING	String
#endif
class Long;
class Float;
class Tuple;
class List;
class Dict;

/**
 *
 */
class Object
{
// construction/destruction
public:

	Object(PyObject *p_pyobj = nullptr);
	Object(Object&& that) noexcept;
	~Object() {}

// attrubutes
public:

	// check for the existence of the named attribute
	bool has_attribute(PYSTRING const &pystr) const;
	bool has_attribute(std::string &&name) const;
	bool has_attribute(std::string const &name) const;
	bool has_attribute(char const *name) const;

	// get the value for a named attribute
	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	T get_attribute(std::string const &name)
	{
		PyObject *p_obj = get_attribute(name.c_str());
		return T(p_obj);
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	T get_attribute(char const *name)
	{
		PyObject *p_obj = get_attribute(name);
		return T(p_obj);
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	T get_attribute(PYSTRING const &name)
	{
		PyObject *p_obj = get_attribute((PyObject const*)name);
		return T(p_obj);
	}

	// set the value for a named attribute
	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool set_attribute(std::string const &name, T& value)
	{
		return set_attribute(name.c_str(), (PyObject*)value);
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool set_attribute(char const *name, T& value)
	{
		return set_attribute(name, (PyObject*)value);
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool set_attribute(PYSTRING const &name, T& value)
	{
		return set_attribute((PyObject const*)name, (PyObject*)value);
	}

	// remove a named attribute
	bool remove_attribute(std::string const &name) { return remove_attribute(name.c_str()); }
	bool remove_attribute(PYSTRING const &name);
	virtual bool remove_attribute(char const *name);

	bool is_instance(const Object& cls);
	bool is_callable() const;

	// access to the PyObject...the caller is stealing a reference
	PyObject *get_pyobject() { return m_ptr_object.get(); }
	PyObject const *get_pyobject() const { return m_ptr_object.get(); }

// operations
public:

#if PY_MAJOR_VERSION < 3
	int compare(Object&& other);
	int compare(const Object& other);
#endif /* PY_MAJOR_VERSION < 3 */

	// invoke a callable object
	template<typename... Args>
	Object invoke(Args&... args);

	// invoke a callable object
	template<typename... Args>
	Object invoke(Args&&... args);

	template<typename... Args>
	Object invoke(Args&&... args, Dict& kw_args);

	template<typename... Args>
	Object invoke(Args&&... args, Dict&& kw_args);

	Object invoke();

	// move
	Object& operator =(Object&& that) noexcept;

	// same as reset
	virtual Object& operator =(PyObject *p_obj);

	// raw python object
	virtual operator PyObject *() { return m_ptr_object.get(); }
	virtual operator PyObject const *() const { return m_ptr_object.get(); }

	// wrap a new PyObject, DECREFs the current PyObject (if set)
	// before adopting the new PyObject
	virtual bool reset(PyObject *p_obj);

	// test for a non-null PyObject
	operator bool() const { return bool(m_ptr_object); }

// implementation
protected:

	// attribute getter implementation
	virtual PyObject *get_attribute(char const *name);
	virtual PyObject *get_attribute(PyObject const *name);
 
	// attribute setter implementation
	virtual bool set_attribute(char const *name, PyObject *p_obj);
	virtual bool set_attribute(PyObject const *name, PyObject *p_obj);

	// attribute deletion implementation
	virtual bool remove_attribute(PyObject const *name);

	// reference to the PyObject*
	PyObjectPtr m_ptr_object;
};

/**
 *
 *
 */
class None : public Object
{
public:

	None();
	None(None&& that);
	~None() {}

// operations
public:

	None& operator =(None&& that);
	Object& operator =(PyObject *p_obj) override;
};

#if defined(PY_MAJOR_VERSION) && (PY_MAJOR_VERSION < 3)
/**
 *
 *
 */
class Int : public Object
{
public:

	Int();
	Int(int v);
	Int(Int&& that);
	Int(PyObject *p_obj);
	~Int() {}

public:

	Int& operator =(const int v) { m_ptr_object.reset(PyInt_FromLong(long(v))); return *this; }
	Int& operator =(Int&& that);
	Object& operator =(PyObject *p_obj) override;

	operator int() const { return int(PyInt_AsLong(const_cast<PyObject*>(m_ptr_object.get()))); }
	operator long() const { return PyInt_AsLong(const_cast<PyObject*>(m_ptr_object.get())); }

	bool reset(PyObject *p_obj);
};
#endif  /* defined(PY_MAJOR_VERSION) && (PY_MAJOR_VERSION < 3) */

/**
 *
 *
 */
class Long : public Object
{
public:

	Long();
	Long(long const v);
	Long(Long&& that);
	Long(PyObject *p_obj);
	~Long() {}

public:

	Long& operator =(double const v) { m_ptr_object.reset(PyLong_FromLong(v)); return *this; }
	Long& operator =(Long&& that);
	Object& operator =(PyObject *p_obj) override;

	operator long() const { return PyLong_AsLong(const_cast<PyObject*>(m_ptr_object.get())); }

	bool reset(PyObject *p_obj);
};

#if PY_MAJOR_VERSION >= 3
using Int = Long;
#endif

/**
 *
 */
class Float : public Object
{
public:

	Float();
	Float(double const v);
	Float(Float&& that);
	Float(PyObject *p_obj);
	~Float() {}

public:

	Float& operator =(double const v);
	Float& operator =(Float&& that);
	Object& operator =(PyObject *p_obj) override;

	operator double() const;

	bool reset(PyObject *p_obj) override;
};

/**
 *
 */
class Complex : public Object
{
public:

	Complex();
	Complex(double const r, double const i);
	Complex(Complex&& that);
	Complex(PyObject *p_obj);
	~Complex() {}

public:

	double real() const;
	double imag() const;
	Object& operator =(PyObject *p_obj) override;

	Complex& operator =(Complex&& that);

	bool reset(PyObject *p_obj) override;
};

/**
 *
 */
class Sequence : public Object
{
public:

	Sequence();
	Sequence(Sequence&& that);
	Sequence(PyObject *p_obj);
	~Sequence() {}

// attributes
public:

	// number of elements in the sequence
	Py_ssize_t size() const;

// operations
public:

	// get an item from the sequence
	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	T get_item(Py_ssize_t pos)
	{
		PyObject *p_obj = get_owned_item(pos);
		return T(p_obj);
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool get_item(Py_ssize_t pos, T& item)
	{
		return get_owned_item(pos, item);
	}

	// set an item in the sequence
	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool set_item(T& item, Py_ssize_t pos)
	{
		return set_owned_item(static_cast<PyObject*>(item), pos);
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool set_item(T&& item, Py_ssize_t pos)
	{
		return set_owned_item(static_cast<PyObject*>(item), pos);
	}

	// remove the item at pos from the sequence
	bool remove_item(Py_ssize_t pos);

	// set a value to each item in the given range
	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	void set_slice(Py_ssize_t i1, Py_ssize_t i2, T& value)
	{
		set_slice(i1, i2, static_cast<PyObject*>(value));
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	void set_slice(Py_ssize_t i1, Py_ssize_t i2, T&& value)
	{
		set_slice(i1, i2, static_cast<PyObject*>(value));
	}
	
	// remove the items
	bool remove_slice(Py_ssize_t i1, Py_ssize_t i2);

	// check for the given value existing in the sequence
	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool contains(const T& value)
	{
		return contains(static_cast<PyObject const*>(value));
	}

	// get the index in the sequence for the given value
	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	Py_ssize_t index(const T& value)
	{
		return get_index(static_cast<PyObject const*>(value));
	}

	// count the number of items in the sequence which are equal to the given value
	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	Py_ssize_t count(const T& value)
	{
		return count(static_cast<PyObject const*>(value));
	}

	// count the number of items in the sequence which are equal to the given value
	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	Py_ssize_t count(T&& value)
	{
		return count(static_cast<PyObject const*>(value));
	}

	Sequence get_slice(Py_ssize_t i1, Py_ssize_t i2) const;

	Sequence& operator =(Sequence&& that);
	Object& operator =(PyObject *p_obj) override;

	bool reset(PyObject *p_obj) override;

// implementation
protected:

	bool get_owned_item(Py_ssize_t pos, Object& item);
	bool contains(const PyObject *p_obj);

	Py_ssize_t get_index(PyObject const *p_obj);
	Py_ssize_t count(PyObject const *p_obj);

	virtual PyObject *get_owned_item(Py_ssize_t pos);
	virtual bool set_owned_item(PyObject *p_obj, Py_ssize_t pos);

	void set_slice(Py_ssize_t i1, Py_ssize_t i2, PyObject *p_obj);
};

#if PY_MAJOR_VERSION < 3
/**
 *
 */
class String : public Sequence
{
public:

	String();
	explicit String(char const *v);
	String(std::string&& v);
	explicit String(std::string const &v);
	String(String&& that);
	String(PyObject *p_obj);
	~String() {}

// operations
public:

	std::string to_string() const;

	operator const char*() const;

	String& operator =(String&& that);
	Object& operator =(PyObject *p_obj) override;

	bool reset(PyObject *p_obj) override;
};

String make_string(const char *format, ...);
#endif /* PY_MAJOR_VERSION < 3 */

/**
 *
 */
class Unicode : public Object
{
// construction/destruction
public:

	Unicode();
	Unicode(char const *str);
	Unicode(char const *str, Py_ssize_t size);
	Unicode(std::string const &str);
	Unicode(std::string&& str);
	Unicode(Unicode&& that) noexcept;
	Unicode(PyObject *p_obj);
	~Unicode() {}

// attributes
public:

	Py_ssize_t size() const;
};

Unicode make_unicode(const char *format, ...);

/**
 *
 */
class Tuple : public Sequence
{
// construction/destruction
public:

	Tuple(Py_ssize_t len);
	Tuple(Tuple&& that);
	Tuple(PyObject *p_pytuple);
	~Tuple() {}

// operations
public:

	Tuple get_slice(Py_ssize_t low, Py_ssize_t high);

	Tuple& operator =(Tuple&& that);
	Object& operator =(PyObject *p_obj) override;

	bool reset(PyObject *p_obj) override;

// implementation
protected:

	bool set_owned_item(PyObject *p_obj, Py_ssize_t pos) override;
};

template<typename T>
void add_tuple_item(Tuple& tuple, Py_ssize_t pos, T&& item)
{
	tuple.set_item(item, pos);
}

template<typename T, typename N, typename... T_Args>
void add_tuple_item(Tuple& tuple, Py_ssize_t pos, T&& item, N&& next, T_Args&&... args)
{
	tuple.set_item(item, pos);
	add_tuple_item(tuple, pos + 1, next, std::forward<T_Args>(args)...);
}

template<typename... T_Args>
Tuple make_tuple(T_Args&&... args)
{
	Tuple tuple(sizeof...(T_Args));
	add_tuple_item(tuple, 0, std::forward<T_Args>(args)...);
	return tuple;
}

/**
 *
 *
 */
class List : public Sequence
{
public:

	List(Py_ssize_t len = 0);
	List(List&& that);
	List(PyObject *p_obj);
	~List() {}

// operations
public:

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	void insert_item(T& item, Py_ssize_t index)
	{
		insert(item, index);
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	void append_item(T& item)
	{
		append(item);
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	void append_item(T&& item)
	{
		append(item);
	}

	List get_slice(Py_ssize_t low, Py_ssize_t high) const;

	Tuple as_tuple() const;

	List& operator =(List&& that);
	Object& operator =(PyObject *p_obj) override;

	bool reset(PyObject *p_obj) override;

// implementation
protected:

	int insert(PyObject *p_obj, Py_ssize_t index);
	int append(PyObject *p_obj);
};


template<typename T>
void add_list_item(List& list, T&& item)
{
	list.append_item(item);
}

template<typename T, typename N, typename... T_Args>
void add_list_item(List& list, T&& item, N&& next, T_Args&&... args)
{
	list.append_item(item);
	add_list_item(list, next, args...);
}

template<typename... T_Args>
List make_list(T_Args&&... args)
{
	List list;
	add_list_item(list, std::forward<T_Args>(args)...);
	return list;
}

/**
 *
 *
 */
class Set : public Object
{
// construction/destruction
public:

	Set();
	Set(Set&& that);
	Set(PyObject *p_obj);
	~Set() {}

// attributes
public:

	Py_ssize_t size() const;

// operations
public:

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool contains(T const &key) const
	{
		return contains_(static_cast<PyObject const*>(key));
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool contains(T&& key) const
	{
		return contains_(static_cast<PyObject const*>(key));
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool add(const T& key)
	{
		return(0 != add_(static_cast<PyObject const*>(key)));
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool add(T&& key)
	{
		return(0 != add_(static_cast<PyObject const*>(key)));
	}

	template<typename V, class = typename std::enable_if<std::is_base_of<Object,V>::value>::type>
	V pop()
	{
		return V(pop_());
	}
	
	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool discard(const T& key)
	{
		return(0 != discard_(static_cast<PyObject const*>(key)));
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool discard(T&& key)
	{
		return(0 != discard_(static_cast<PyObject const*>(key)));
	}

	int clear();

	bool reset(PyObject *p_obj) override;

protected:

	bool contains_(const PyObject *p_obj) const;
	int add_(const PyObject *p_obj);
	PyObject *pop_();
	int discard_(const PyObject* p_obj);
};

/**
 *
 *
 */
class Dict : public Object
{
// construction/destruction
public:

	Dict();
	Dict(Dict&& that);
	Dict(const Dict& that);
	Dict(PyObject *p_obj);
	~Dict() {}

// operations
public:

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool contains(T const &key)
	{
		return contains((const PyObject*)key);
	}

	template<typename T, class = typename std::enable_if<std::is_base_of<Object,T>::value>::type>
	bool contains(T&& key)
	{
		return contains(static_cast<PyObject const*>(key));
	}

	// set a key/value pair in the dictionary
	// both types K and V must be Object derived
	template<
		typename K, typename V,
		class = typename std::enable_if<std::is_base_of<Object,K>::value>::type,
		class = typename std::enable_if<std::is_base_of<Object,V>::value>::type>
	bool set_item(K const &key, V const &value)
	{
		const PyObject *p_key = static_cast<PyObject const*>(key);
		const PyObject *p_value = static_cast<PyObject const*>(value);
		return set_owned_item(p_key, p_value);
	}

	// set a key/value pair in the dictionary
	// both types K and V must be Object derived
	template<
		typename K, typename V,
		class = typename std::enable_if<std::is_base_of<Object,K>::value>::type,
		class = typename std::enable_if<std::is_base_of<Object,V>::value>::type>
	bool set_item(K const &key, V&& value)
	{
		return set_owned_item(static_cast<PyObject const*>(key), static_cast<PyObject*>(value));
	}

	template<typename V, class = typename std::enable_if<std::is_base_of<Object,V>::value>::type>
	bool set_item(const char *key, const V& value)
	{
		return set_owned_item(key, (const PyObject*)value);
	}

	template<typename V, class = typename std::enable_if<std::is_base_of<Object,V>::value>::type>
	bool set_item(char const *key, V&& value)
	{
		return set_owned_item(key, static_cast<PyObject*>(value));
	}

	// look up a value in the dictionary for the given key
	// both types K and V must be Object derived
	template<
		typename V, typename K,
		class = typename std::enable_if<std::is_base_of<Object,K>::value>::type,
		class = typename std::enable_if<std::is_base_of<Object,V>::value>::type>
	V get_item(K const &key)
	{
		PyObject *p_obj = get_owned_item(static_cast<PyObject const*>(key));
		if(nullptr == p_obj)
			return V();
		return V(p_obj);
	}

	// look up a value in the dictionary for the given key
	// both types K and V must be Object derived
	template<
		typename V, typename K,
		class = typename std::enable_if<std::is_base_of<Object,K>::value>::type,
		class = typename std::enable_if<std::is_base_of<Object,V>::value>::type>
	V get_item(K&& key)
	{
		PyObject *p_obj = get_owned_item(static_cast<PyObject const*>(key));
		if(nullptr == p_obj)
			return V();
		return V(p_obj);
	}

	template<typename V, class = typename std::enable_if<std::is_base_of<Object,V>::value>::type>
	V get_item(char const *key)
	{
		PyObject *p_obj = get_owned_item(key);
		if(nullptr == p_obj)
			return V();
		return V(p_obj);
	}

	void clear();

	bool reset(PyObject *p_obj) override;

protected:

	bool contains(const PyObject *p_obj);

	bool set_owned_item(const PyObject *p_key, const PyObject *p_value);
	bool set_owned_item(const char *key, const PyObject *p_value);

	PyObject *get_owned_item(const PyObject *p_key);
	PyObject *get_owned_item(const char *p_key);
};

template<typename K, typename V>
void add_dict_entry(Dict& dict, K& key, V& value)
{
	dict.set_item(key, value);
}

template<typename K1, typename V1, typename K2, typename V2, typename... T_Args>
void add_dict_entry(Dict& dict, K1& key, V1& value, K2& key2, V2& value2, T_Args&&... args)
{
	dict.set_item(key, value);
	add_dict_entry(dict, key2, value2, std::forward<T_Args&>(args)...);
}

template<typename... T_Args>
Dict make_dict(T_Args&&... args)
{
	Dict dict;
	add_dict_entry(dict, std::forward<T_Args&>(args)...);
	return dict;
}

// -- Object invoke methods ----------------------------------------------------

template<typename... Args>
Object Object::invoke(Args&... args)
{
	Tuple py_args = make_tuple(std::forward<Args>(args)...);
	PyObject *p_obj = PyInvokeMethod(PyObject_CallObject, m_ptr_object.get(), static_cast<PyObject*>(py_args));
	if(p_obj == nullptr)
		return None();
	return Object(p_obj);
}

template<typename... Args>
Object Object::invoke(Args&&... args)
{
	Tuple py_args = make_tuple(std::forward<Args>(args)...);
	PyObject *p_obj = PyInvokeMethod(PyObject_CallObject, m_ptr_object.get(), static_cast<PyObject*>(py_args));
	if(p_obj == nullptr)
		return None();
	return Object(p_obj);
}

template<typename... Args>
Object Object::invoke(Args&&... args, Dict& kw_args)
{
	Tuple py_args = make_tuple(std::forward<Args>(args)...);
	PyObject *p_obj = PyInvokeMethod(PyObject_Call, m_ptr_object.get(), static_cast<PyObject*>(py_args), static_cast<PyObject*>(kw_args));
	return Object(p_obj);
}

template<typename... Args>
Object Object::invoke(Args&&... args, Dict&& kw_args)
{
	Tuple py_args = make_tuple(std::forward<Args>(args)...);
	PyObject *p_obj = PyInvokeMethod(PyObject_Call, m_ptr_object.get(), static_cast<PyObject*>(py_args), static_cast<PyObject*>(kw_args));
	return Object(p_obj);
}

// -----------------------------------------------------------------------------

/**
 *
 *
 */
class Module : public Object
{
// construction/destruction
public:

	Module();
	Module(Module&& that) noexcept;
	Module(PyObject *p_obj);
	~Module() {}

// attributes
public:

	std::string name();
	Dict dict();

// operations
public:

	bool add_object(const char *name, PyTypeObject *p_type);
	
	virtual Module& operator =(Module&& that) noexcept;
	Object& operator =(PyObject *p_obj) override;

	bool reset(PyObject *p_obj) override;

	static Module import(const std::string& name);
};

/**
 * PySys_* functions
 *
 */
struct Sys
{
	// get the named system object
	static PyObject *get_object(const char *name);
};

}   // namespace PyWrap

#endif  /* __PYTHON_TYPES_HEADER__ */
