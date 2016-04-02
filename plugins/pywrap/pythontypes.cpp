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
#include "pythontypes.h"
#include "pythonutil.h"

namespace PyWrap {

#ifdef DEBUG_PYREF
void PythonDecref::operator ()(PyObject* p_pyobj)
{
	std::cout << "PyObject* " << (void*)p_pyobj << " is at REFCNT " << Py_REFCNT(p_pyobj) << std::endl;
	if(Py_REFCNT(p_pyobj) > 0)
		Py_DECREF(p_pyobj);
}
#endif

Object::Object(Object&& that) noexcept
:   m_ptr_object(std::move(that.m_ptr_object))
{
#ifdef DEBUG_PYREF
	if(m_ptr_object)
	{
		std::cout
			<< "Object::Object(move): PyObject* "
			<< (void*)m_ptr_object.get()
			<< " is at REFCNT "
			<< Py_REFCNT(m_ptr_object.get())
			<< std::endl;
	}
#endif
}

Object::Object(PyObject *p_pyobj)
:   m_ptr_object(p_pyobj)
{
	if(m_ptr_object && (Py_REFCNT(p_pyobj) == 0))
	{
#ifdef DEBUG_PYREF
		std::cout << "Object::Object: PyObject* " << (void*)p_pyobj << " has a ZERO refcount" << std::endl;
#endif
		Py_INCREF(p_pyobj);
#ifdef DEBUG_PYREF
		std::cout << "Object::Object: PyObject* " << (void*)p_pyobj << " is at REFCNT " << Py_REFCNT(p_pyobj) << std::endl;
#endif
	}
}

#if PY_MAJOR_VERSION < 3
int Object::compare(Object&& other)
{
	int result = PyInvokeMethod(PyObject_Compare, m_ptr_object.get(), static_cast<PyObject*>(other));
	return result;
}

int Object::compare(const Object& other)
{
	int result = PyInvokeMethod(PyObject_Compare, m_ptr_object.get(), static_cast<PyObject*>(const_cast<Object&>(other)));
	return result;
}
#endif /* PY_MAJOR_VERSION < 3 */

Object Object::invoke()
{
	PyObject *p_obj = PyInvokeMethod(PyObject_CallObject, m_ptr_object.get(), static_cast<PyObject*>(nullptr));
	if(p_obj == nullptr)
		return None();
	return Object(p_obj);
}

Object& Object::operator =(Object&& that) noexcept
{
	m_ptr_object = std::move(that.m_ptr_object);
	return *this;
}

Object& Object::operator =(PyObject *p_obj)
{
	m_ptr_object.reset(p_obj);
	return *this;
}

bool Object::reset(PyObject *p_obj)
{
	m_ptr_object.reset(p_obj);
	return bool(m_ptr_object);
}

bool Object::has_attribute(PYSTRING const &pystr) const
{
	bool result = false;
	if(m_ptr_object)
	{
		// python docs claim this call always succeeds
		result = (1 == PyObject_HasAttr(m_ptr_object.get(), static_cast<PyObject*>(const_cast<PYSTRING&>(pystr))));
	}
	return result;
}

bool Object::has_attribute(std::string &&name) const
{
	return has_attribute(name.c_str());
}

bool Object::has_attribute(std::string const &name) const
{
	return has_attribute(name.c_str());
}

bool Object::has_attribute(char const *name) const
{
	bool result = false;
	if(m_ptr_object)
	{
		// python docs claim this call always succeeds
		result = (1 == PyObject_HasAttrString(m_ptr_object.get(), name));
	}
	return result;
}

bool Object::is_instance(Object const &cls)
{
	int status = PyInvokeMethod(PyObject_IsInstance, m_ptr_object.get(), static_cast<PyObject*>(const_cast<Object&>(cls)));
	return(1 == status);
}

bool Object::is_callable() const
{
	// python docs claim this function always succeedes
	int status = PyCallable_Check(const_cast<PyObject*>(m_ptr_object.get()));
	return(1 == status);
}

PyObject *Object::get_attribute(char const *name)
{
	PyObject *p_obj = PyInvokeMethod(PyObject_GetAttrString, m_ptr_object.get(), name);
	return p_obj;
}

PyObject *Object::get_attribute(PyObject const *name)
{
	PyObject *p_obj = PyInvokeMethod(PyObject_GetAttr, m_ptr_object.get(), const_cast<PyObject*>(name));
	return p_obj;
}

bool Object::set_attribute(char const *name, PyObject *p_obj)
{
	int status = PyInvokeMethod(PyObject_SetAttrString, m_ptr_object.get(), name, p_obj);
	return(-1 != status);
}

bool Object::set_attribute(PyObject const *name, PyObject *p_obj)
{
	int status = PyInvokeMethod(PyObject_SetAttr, m_ptr_object.get(), const_cast<PyObject*>(name), p_obj);
	return(-1 != status);
}

bool Object::remove_attribute(PYSTRING const &name)
{
	return remove_attribute(static_cast<PyObject const*>(name));
}

bool Object::remove_attribute(char const *name)
{
	return set_attribute(name, nullptr);
}

bool Object::remove_attribute(PyObject const *name)
{
	return set_attribute(name, nullptr);
}

None::None()
{
	Py_INCREF(Py_None);
	m_ptr_object.reset(Py_None);
}

None::None(None&& that)
:	Object(std::move(that))
{
}

None& None::operator =(None&& that)
{
	Object::operator =(std::move(that));
	return *this;
}

Object& None::operator =(PyObject *p_obj)
{
	if((p_obj == nullptr) || (p_obj != Py_None))
		throw Error("None::operator =: attempt to set to other than Py_None");
	m_ptr_object.reset(p_obj);
	return Object::operator =(p_obj);
}

#if defined(PY_MAJOR_VERSION) && (PY_MAJOR_VERSION < 3)
// Error type for PyInt_Check failures
class NotIntErr : public Error
{
public:

	NotIntErr(char const *prefix)
		: Error(prefix + std::string(": PyInt_Check != true")) {}
	~NotIntErr() {}
};

Int::Int()
:   Object(PyInvokeFunc(PyInt_FromLong, 0L))
{
}

Int::Int(int v)
:   Object(PyInvokeFunc(PyLong_FromLong, long(v)))
{
}

Int::Int(Int&& that)
:   Object(std::move(that))
{
}

Int::Int(PyObject *p_obj)
:   Object(p_obj)
{
	if(m_ptr_object && !PyInt_Check(p_obj))
		throw NotIntErr("Int::Int");
}

Int& Int::operator =(Int&& that)
{
	Object::operator =(std::move(that));
	return *this;
}

Object& Int::operator =(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyInt_Check(p_obj))
		throw NotIntErr("Int::operator =");
	return Object::operator =(p_obj);
}

bool Int::reset(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyInt_Check(p_obj))
		return false;
	return Object::reset(p_obj);
}
#endif	/* defined(PY_MAJOR_VERSION) && (PY_MAJOR_VERSION < 3) */

// Error type for PyLong_Check failures
class NotLongErr : public Error
{
public:

	NotLongErr(char const *prefix)
		: Error(prefix + std::string(": PyLong_Check != true")) {}
	~NotLongErr() {}
};

Long::Long()
:   Object(PyInvokeFunc(PyLong_FromLong, long(0)))
{
}

Long::Long(const long v)
:   Object(PyInvokeFunc(PyLong_FromLong, v))
{
}

Long::Long(Long&& that)
:   Object(std::move(that))
{
}

Long::Long(PyObject *p_obj)
:   Object(p_obj)
{
	if(m_ptr_object && !PyLong_Check(p_obj))
		throw NotLongErr("Long::Long");
}

Long& Long::operator =(Long&& that)
{
	Object::operator =(std::move(that));
	return *this;
}

Object& Long::operator =(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyLong_Check(p_obj))
		throw NotLongErr("Long::operator =");
	return Object::operator =(p_obj);
}

bool Long::reset(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyLong_Check(p_obj))
		return false;
	return Object::reset(p_obj);
}

// Error type for PyFloat_Check failures
class NotFloatErr : public Error
{
public:

	NotFloatErr(char const *prefix)
		: Error(prefix + std::string(": PyFloat_Check != true")) {}
	~NotFloatErr() {}
};

Float::Float()
:   Object(PyInvokeFunc(PyFloat_FromDouble, 0.0))
{
}

Float::Float(double const v)
:   Object(PyInvokeFunc(PyFloat_FromDouble, v))
{
}

Float::Float(Float&& that)
:   Object(std::move(that))
{
}

Float::Float(PyObject *p_obj)
:   Object(p_obj)
{
	if(m_ptr_object && !PyFloat_Check(p_obj))
		throw NotFloatErr("Float::FLoat");
}

Float& Float::operator =(double const v)
{
	PyObject *p_obj = PyInvokeFunc(PyFloat_FromDouble, v);
	m_ptr_object.reset(p_obj);
	return *this;
}

Float& Float::operator =(Float&& that)
{
	Object::operator =(std::move(that));
	return *this;
}

Object& Float::operator =(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyLong_Check(p_obj))
		throw NotFloatErr("Float::operator =");
	return Object::operator =(p_obj);
}

Float::operator double() const
{
	double result = PyInvokeMethod(PyFloat_AsDouble, m_ptr_object.get());
	return result;
}

bool Float::reset(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyFloat_Check(p_obj))
		return false;
	return Object::reset(p_obj);
}

// Error type for PyComplex_Check failures
class NotComplexErr : public Error
{
public:

	NotComplexErr(char const *prefix)
		: Error(prefix + std::string(": PyComplex_Check != true")) {}
	~NotComplexErr() {}
};

Complex::Complex()
:   Complex(0.0, 0.0)
{
}

Complex::Complex(double const r, double const i)
:   Object(PyInvokeFunc(PyComplex_FromDoubles, r, i))
{
}

Complex::Complex(Complex&& that)
:   Object(std::move(that))
{
}

Complex::Complex(PyObject *p_obj)
:   Object(p_obj)
{
	if(m_ptr_object)
	{
		if(!PyComplex_Check(p_obj))
			throw NotComplexErr("Complex::Complex");
	}
}

double Complex::real() const
{
	double result = PyInvokeMethod(PyComplex_RealAsDouble, m_ptr_object.get());
	return result;
}

double Complex::imag() const
{
	double result = PyInvokeMethod(PyComplex_ImagAsDouble, m_ptr_object.get());
	return result;
}

Complex& Complex::operator =(Complex&& that)
{
	Object::operator =(std::move(that));
	return *this;
}

Object& Complex::operator =(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyLong_Check(p_obj))
		throw NotComplexErr("Complex::operator =");
	return Object::operator =(p_obj);
}

bool Complex::reset(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyComplex_Check(p_obj))
		return false;
	return Object::reset(p_obj);
}

// Error type for PySequence_Check failures
class NotSeqErr : public Error
{
public:

	NotSeqErr(char const *prefix)
		: Error(prefix + std::string(": PySequence_Check != true")) {}
	~NotSeqErr() {}
};

Sequence::Sequence()
{
}

Sequence::Sequence(Sequence&& that)
:	Object(std::move(that))
{
}

Sequence::Sequence(PyObject *p_obj)
:	Object(p_obj)
{
	if(m_ptr_object && !PySequence_Check(p_obj))
		throw NotSeqErr("Sequence::Sequence");
}

Py_ssize_t Sequence::size() const
{
	Py_ssize_t result = PyInvokeMethod(PySequence_Size, m_ptr_object.get());
	return result;
}

bool Sequence::remove_item(Py_ssize_t pos)
{
	int status = PyInvokeMethod(PySequence_DelItem, m_ptr_object.get(), pos);
	return(-1 != status);
}

bool Sequence::remove_slice(Py_ssize_t i1, Py_ssize_t i2)
{
	int status = PyInvokeMethod(PySequence_DelSlice, m_ptr_object.get(), i1, i2);
	return(-1 != status);
}

Sequence Sequence::get_slice(Py_ssize_t i1, Py_ssize_t i2) const
{
	PyObject *p_obj = PyInvokeMethod(PySequence_GetSlice, const_cast<PyObject*>(m_ptr_object.get()), i1, i2);
	return Sequence(p_obj);
}

Sequence& Sequence::operator =(Sequence&& that)
{
	Object::operator =(std::move(that));
	return *this;
}

Object& Sequence::operator =(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PySequence_Check(p_obj))
		throw NotSeqErr("Sequence::operator =");
	return Object::operator =(p_obj);
}

bool Sequence::reset(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PySequence_Check(p_obj))
		return false;
	return Object::reset(p_obj);
}

bool Sequence::get_owned_item(Py_ssize_t pos, Object& item)
{
	PyObject *p_obj = get_owned_item(pos);
	if(p_obj == nullptr)
		return false;
	return item.reset(p_obj);
}

bool Sequence::set_owned_item(PyObject *p_obj, Py_ssize_t pos)
{
	int status = PyInvokeMethod(PySequence_SetItem, m_ptr_object.get(), pos, p_obj);
	return(status != -1);
}

bool Sequence::contains(const PyObject *p_obj)
{
	int status = PyInvokeMethod(PySequence_Contains, m_ptr_object.get(), const_cast<PyObject*>(p_obj));
	return(1 == status);
}

Py_ssize_t Sequence::get_index(const PyObject *p_obj)
{
	Py_ssize_t index = PyInvokeMethod(PySequence_Index, m_ptr_object.get(), const_cast<PyObject*>(p_obj));
	return index;
}

Py_ssize_t Sequence::count(PyObject const *p_obj)
{
	Py_ssize_t result = PyInvokeMethod(PySequence_Count, m_ptr_object.get(), const_cast<PyObject*>(p_obj));
	return result;
}

PyObject *Sequence::get_owned_item(Py_ssize_t pos)
{
	PyObject *p_obj = PyInvokeMethod(PySequence_GetItem, m_ptr_object.get(), pos);
	return p_obj;
}

void Sequence::set_slice(Py_ssize_t i1, Py_ssize_t i2, PyObject *p_obj)
{
	PyInvokeMethod(PySequence_SetSlice, m_ptr_object.get(), i1, i2, p_obj);
}

#if PY_MAJOR_VERSION < 3
// Error type for PyString_Check failures
class NotStringErr : public Error
{
public:

	NotStringErr(char const *prefix)
		: Error(prefix + std::string(": PyString_Check != true")) {}
	~NotStringErr() {}
};

String::String()
:	Sequence(PyInvokeFunc(PyString_FromString, ""))
{
}

String::String(const char *v)
:	Sequence(PyInvokeFunc(PyString_FromString, v))
{
}

String::String(std::string&& v)
:	Sequence(PyInvokeFunc(PyString_FromString, v.c_str()))
{
}

String::String(const std::string& v)
:	Sequence(PyInvokeFunc(PyString_FromString, v.c_str()))
{
}

String::String(String&& that)
:	Sequence(std::move(that))
{
}

String::String(PyObject *p_obj)
:	Sequence(p_obj)
{
	if(m_ptr_object && !PyString_Check(p_obj))
		throw NotStringErr("String::String");
}

std::string String::to_string() const
{
	return std::string(this->operator char const*());
}

String::operator const char*() const
{
	char *result = PyInvokeMethod(PyString_AsString, m_ptr_object.get());
	return result;
}

String& String::operator =(String&& that)
{
	Sequence::operator =(std::move(that));
	return *this;
}

Object& String::operator =(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyString_Check(p_obj))
		throw NotStringErr("String::operator =");
	return Object::operator =(p_obj);
}

bool String::reset(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyString_Check(p_obj))
		return false;
	return Sequence::reset(p_obj);
}

String make_string(char const *format, ...)
{
	PyObject *p_obj;
	va_list vargs;
	va_start(vargs, format);
	try { p_obj = PyInvokeFunc(PyString_FromFormatV, format, vargs); }
	catch(...)
	{
		va_end(vargs);
		throw;
	}
	va_end(vargs);

	return String(p_obj);
}
#endif /* PY_MAJOR_VERSION < 3 */


// Error type for PyUnicode_Check failures
class NotUnicodeErr : public Error
{
public:

	NotUnicodeErr(char const *prefix)
		: Error(prefix + std::string(": PyUnicode_Check != true")) {}
	~NotUnicodeErr() {}
};

Unicode::Unicode()
{
}

Unicode::Unicode(const char *str)
:	Object(PyInvokeFunc(PyUnicode_FromString, str))
{
}

Unicode::Unicode(const char *str, Py_ssize_t size)
:	Object(PyInvokeFunc(PyUnicode_FromStringAndSize, str, size))
{
}

Unicode::Unicode(std::string const &str)
:	Object(PyInvokeFunc(PyUnicode_FromString, str.c_str()))
{
}

Unicode::Unicode(std::string&& str)
:	Object(PyInvokeFunc(PyUnicode_FromString, str.c_str()))
{
}

Unicode::Unicode(Unicode&& that) noexcept
:	Object(std::move(that))
{
}

Unicode::Unicode(PyObject *p_obj)
:	Object(p_obj)
{
	if(m_ptr_object && !PyUnicode_Check(p_obj))
		throw NotUnicodeErr("Unicode::Unicode");
}

Py_ssize_t Unicode::size() const
{
	Py_ssize_t result =
#if PY_MAJOR_VERSION < 3
		PyInvokeMethod(PyUnicode_GetSize, m_ptr_object.get());
#else
		//PyInvokeMethod(PyUnicode_GET_LENGTH, m_ptr_object.get());
		PyUnicode_GET_LENGTH(m_ptr_object.get());
		check_pyerr();
#endif

	return result;
}

Unicode make_unicode(const char *format, ...)
{
	PyObject *p_obj;
	va_list vargs;
	va_start(vargs, format);
	try { p_obj = PyInvokeFunc(PyUnicode_FromFormatV, format, vargs); }
	catch(...)
	{
		va_end(vargs);
		throw;
	}
	va_end(vargs);

	return Unicode(p_obj);
}

// Error type for PyTuple_Check failures
class NotTupleErr : public Error
{
public:

	NotTupleErr(char const *prefix)
		: Error(prefix + std::string(": PyTuple_Check != true")) {}
	~NotTupleErr() {}
};

Tuple::Tuple(Py_ssize_t len)
:   Sequence(PyInvokeFunc(PyTuple_New, len))
{
}

Tuple::Tuple(Tuple&& that)
:	Sequence(std::move(that))
{
}

Tuple::Tuple(PyObject *p_pytuple)
:   Sequence(p_pytuple)
{
	if(m_ptr_object && !PyTuple_Check(p_pytuple))
		throw NotTupleErr("Tuple::Tuple");
}

Tuple Tuple::get_slice(Py_ssize_t low, Py_ssize_t high)
{
	return Tuple(PyInvokeMethod(PyTuple_GetSlice, m_ptr_object.get(), low, high));
}

Tuple& Tuple::operator =(Tuple&& that)
{
	m_ptr_object = std::move(that.m_ptr_object);
	return *this;
}

Object& Tuple::operator =(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyTuple_Check(p_obj))
		throw NotTupleErr("Tuple::operator =");
	return Object::operator =(p_obj);
}

bool Tuple::reset(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyTuple_Check(p_obj))
		return false;
	return Object::reset(p_obj);
}

bool Tuple::set_owned_item(PyObject *p_obj, Py_ssize_t pos)
{
	// PyTyple_SetItem steals a reference
	bool result = 0 == PyInvokeMethod(PyTuple_SetItem, m_ptr_object.get(), pos, p_obj);

	if(result) Py_INCREF(p_obj);

	return result;
}

// Error type for PyList_Check failures
class NotListErr : public Error
{
public:

	NotListErr(char const *prefix)
		: Error(prefix + std::string(": PyList_Check != true")) {}
	~NotListErr() {}
};

List::List(Py_ssize_t len/* = 0*/)
:	Sequence(PyInvokeFunc(PyList_New, len))
{

}

List::List(List&& that)
:	Sequence(std::move(that))
{
}

List::List(PyObject *p_obj)
:	Sequence(p_obj)
{
	if(m_ptr_object && !PyList_Check(p_obj))
		throw NotListErr("List::List");
}

List List::get_slice(Py_ssize_t low, Py_ssize_t high) const
{
	PyObject *p_obj = PyInvokeMethod(PyList_GetSlice, m_ptr_object.get(), low, high);
	return List(p_obj);
}

Tuple List::as_tuple() const
{
	PyObject *p_obj = PyInvokeMethod(PyList_AsTuple, m_ptr_object.get());
	return Tuple(p_obj);
}

List& List::operator =(List&& that)
{
	m_ptr_object = std::move(that.m_ptr_object);
	return *this;
}

Object& List::operator =(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyList_Check(p_obj))
		throw NotListErr("List::operator =");
	return Object::operator =(p_obj);
}

int List::insert(PyObject *p_obj, Py_ssize_t index)
{
	int result = PyInvokeMethod(PyList_Insert, m_ptr_object.get(), index, p_obj);
	return result;
}

int List::append(PyObject *p_obj)
{
	int result = PyInvokeMethod(PyList_Append, m_ptr_object.get(), p_obj);
	return result;
}

bool List::reset(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyList_Check(p_obj))
		return false;
	return Sequence::reset(p_obj);
}

Set::Set()
:   Object(PyInvokeFunc(PySet_New, (PyObject*)(nullptr)))
{
}

Set::Set(Set&& that)
:   Object(std::move(that))
{
}

Set::Set(PyObject *p_obj)
:   Object(p_obj)
{
	if(m_ptr_object)
	{
		if(!PySet_Check(p_obj))
			throw Error("Set::Set: PySet_Check !- true");
	}
}

int Set::clear()
{
	int result = PyInvokeMethod(PySet_Clear, m_ptr_object.get());
	return result;
}

bool Set::reset(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PySet_Check(p_obj))
		return false;
	return Object::reset(p_obj);
}

bool Set::contains_(PyObject const *p_obj) const
{
	bool result = false;
	if(nullptr != p_obj)
		result = (0 != PyInvokeMethod(PySet_Contains, m_ptr_object.get(), const_cast<PyObject*>(p_obj)));
	return result;
}

int Set::add_(PyObject const *p_obj)
{
	int result = PyInvokeMethod(PySet_Add, m_ptr_object.get(), const_cast<PyObject*>(p_obj));
	return result;
}

PyObject *Set::pop_()
{
	PyObject *result = PyInvokeMethod(PySet_Pop, m_ptr_object.get());
	return result;
}

int Set::discard_(PyObject const *p_obj)
{
	int result = PyInvokeMethod(PySet_Discard, m_ptr_object.get(), const_cast<PyObject*>(p_obj));
	return result;
}

Dict::Dict()
:   Object(PyInvokeFunc(PyDict_New))
{
}

Dict::Dict(Dict&& that)
:   Object(std::move(that))
{
}

Dict::Dict(const Dict& that)
:   Object(PyInvokeMethod(PyDict_Copy, m_ptr_object.get()))
{
}

Dict::Dict(PyObject *p_obj)
:   Object(p_obj)
{
	if(!PyDict_Check(p_obj))
		throw Error("Dict: PyDict_Check != true");
}

void Dict::clear()
{
	PyInvokeMethod(PyDict_Clear, m_ptr_object.get());
}

bool Dict::contains(PyObject const *p_obj)
{
	int result = PyInvokeMethod(PyDict_Contains, m_ptr_object.get(), const_cast<PyObject*>(p_obj));
	return(1 == result);
}

bool Dict::reset(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyDict_Check(p_obj))
		return false;
	return Object::reset(p_obj);
}

bool Dict::set_owned_item(PyObject const *p_key, PyObject const *p_value)
{
#ifdef DEBUG_PYREF
	std::cout << "Dict::set_owned_item: pre-set item REFCNT is " << Py_REFCNT(p_value) << std::endl;
#endif
	int result = PyInvokeMethod(PyDict_SetItem, m_ptr_object.get(), const_cast<PyObject*>(p_key), const_cast<PyObject*>(p_value));
#ifdef DEBUG_PYREF
	std::cout << "Dict::set_owned_item: post-set item REFCNT is " << Py_REFCNT(p_value) << std::endl;
#endif
	return(0 == result);
}

bool Dict::set_owned_item(char const *key, PyObject const *p_value)
{
#ifdef DEBUG_PYREF
	std::cout << "Dict::set_owned_item: pre-set item REFCNT is " << Py_REFCNT(p_value) << std::endl;
#endif
	int result = PyInvokeMethod(PyDict_SetItemString, m_ptr_object.get(), key, const_cast<PyObject*>(p_value));
#ifdef DEBUG_PYREF
	std::cout << "Dict::set_owned_item: post-set item REFCNT is " << Py_REFCNT(p_value) << std::endl;
#endif
	return(0 == result);
}

PyObject *Dict::get_owned_item(PyObject const *p_key)
{
	PyObject *p_result = PyInvokeMethod(PyDict_GetItem, m_ptr_object.get(), const_cast<PyObject*>(p_key));
	if(nullptr != p_result)
		Py_INCREF(p_result);
	return p_result;
}

PyObject *Dict::get_owned_item(char const *p_key)
{
	PyObject *p_result = PyInvokeMethod(PyDict_GetItemString, m_ptr_object.get(), p_key);
	if(nullptr != p_result)
		Py_INCREF(p_result);
	return p_result;
}


// Error type for PyModule_Check failures
class NotModErr : public Error
{
public:

	NotModErr(char const *prefix)
		: Error(prefix + std::string(": PyModule_Check != true")) {}
	~NotModErr() {}
};

Module::Module()
{
}

Module::Module(Module&& that) noexcept
:	Object(std::move(that))
{
}

Module::Module(PyObject *p_obj)
:	Object(p_obj)
{
	if(m_ptr_object && !PyModule_Check(p_obj))
		throw NotModErr("Module::Module");
}

std::string Module::name()
{
#if PY_MAJOR_VERSION >= 3
	const 
#endif
	char *name = PyInvokeMethod(PyModule_GetName, m_ptr_object.get());
	return std::string((name != nullptr ? name : ""));
}

Module Module::import(const std::string& name)
{
	PyObject *p_mod = PyInvokeFunc(PyImport_ImportModule, name.c_str());
	return Module(p_mod);
}

Dict Module::dict()
{
	// returns a borrowed reference
	PyObject *p_obj = PyInvokeMethod(PyModule_GetDict, m_ptr_object.get());

	// passing nullptr to INCREF just like a no-op
	return Dict(INCREF(p_obj));
}

bool Module::add_object(const char *name, PyTypeObject *p_type)
{
	int result = PyInvokeMethod(PyModule_AddObject, m_ptr_object.get(), name, INCREF(reinterpret_cast<PyObject*>(p_type)));
	return(0 == result);
}

Module& Module::operator =(Module&& that) noexcept
{
	Object::operator =(std::move(that));
	return *this;
}

Object& Module::operator =(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyModule_Check(p_obj))
		throw NotModErr("Module::operator =");
	return Object::operator =(p_obj);
}

bool Module::reset(PyObject *p_obj)
{
	if((p_obj != nullptr) && !PyModule_Check(p_obj))
		return false;
	return Object::reset(p_obj);
}

PyObject *Sys::get_object(const char *name)
{
#if PY_MAJOR_VERSION >= 3
	PyObject *result = PyInvokeFunc(PySys_GetObject, name);
#else
	PyObject *result = PyInvokeFunc(PySys_GetObject, const_cast<char*>(name));
#endif
	return INCREF(result);
}

}   // namespace PyWrap
