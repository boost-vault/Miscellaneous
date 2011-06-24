#pragma once

//protected_cast library
//
//Overview:
//	This library provides a means for accessing protected class members in a clear fashion,
// similar to the way C++ casts work.
//
//Motivation:
//	Debugging and short-circuiting designs.
//
//Usage:
//protected_cast offers two styles of usage - one inplace, a bit tedious;
// the other allows any expressions, but needs additional preparations.
//
//==Inplace usage==
//Synopsis: BOOST_PROTECTED_CAST_LVALUE(CLASS, MEMBER, INSTANCE)
//Description:
//	Provides mutable lvalue access to 'MEMBER' within 'INSTANCE' of 'CLASS'.
//Example:
//	BOOST_PROTECTED_CAST_LVALUE(SomeClass, m_Member, someInstance) = 15;
//	This example sets the 'm_Member' member variable in 'someInstance' to 15;
//
//Synopsis: BOOST_PROTECTED_CAST_RVALUE(CLASS, MEMBER, INSTANCE, EXPR)
//Description:
//	Provides rvalue access to 'MEMBER' within 'INSTANCE' of 'CLASS'. EXPR is prepended to the MEMBER access spot.
//Example:
//	BOOST_PROTECTED_CAST_RVALUE(SomeClass, m_Member, someInstance, someVar += ) - 5; // someVar += someInstance.m_Member - 5;
//
//==Placeholder usage==
//Synopsis: BOOST_PROTECTED_CAST_PLACEHOLDER(CLASS, MEMBER);
//	later BOOST_PROTECTED_CAST(CLASS, MEMBER, INSTANCE)
//Description:
//	Provides usage of 'MEMBER' within 'INSTANCE' of 'CLASS' as an ordinary identifier.
//Example:
//	BOOST_PROTECTED_CAST_PLACEHOLDER(SomeClass, m_Member);
//...
//	SomeClass thisClass;
//	std::cout << BOOST_PROTECTED_CAST(SomeClass, m_Member, thisClass) << std::endl;

/* Sample:

class badboy
{
public:
	badboy(int x) : a(x) { cout << a << endl; }
protected:
	int a;
};

void main()
{
	BOOST_PROTECTED_CAST_PLACEHOLDER(badboy, a); // required for BOOST_PROTECTED_CAST

	badboy b(7);
	cout << BOOST_PROTECTED_CAST(badboy, a, b) << endl;
	BOOST_PROTECTED_CAST_LVALUE(badboy, a, b) = 17;
	cout << BOOST_PROTECTED_CAST(badboy, a, b) << endl;
	BOOST_PROTECTED_CAST(badboy, a, b) = 7;
	cout << BOOST_PROTECTED_CAST(badboy, a, b) << endl;

	int x;
	BOOST_PROTECTED_CAST_RVALUE(badboy, a, b, x=) + 5; // x = b.a+5
	cout << x << endl; // 12
	BOOST_PROTECTED_CAST_RVALUE(badboy, a, b, x+=3-); // x += 3-b.a
	cout << x << endl; // 8
}

*/

#include <boost/preprocessor/cat.hpp>


#define BOOST_PROTECTED_CAST_PLACEHOLDER(CLASS, MEMBER) \
	struct PROTECTED_CAST_class_##CLASS##_member_##MEMBER : public CLASS { using CLASS::MEMBER; }

#define BOOST_PROTECTED_CAST(CLASS, MEMBER, INSTANCE) \
	static_cast<PROTECTED_CAST_class_##CLASS##_member_##MEMBER*>(&INSTANCE)->MEMBER

#define BOOST_PROTECTED_CAST_LVALUE(CLASS, MEMBER, INSTANCE) \
	struct BOOST_PP_CAT(PROTECTED_CAST_LVALUE_class_##CLASS##_member_##MEMBER,__LINE__): public CLASS \
	{ using CLASS::MEMBER; }; \
	static_cast<BOOST_PP_CAT(PROTECTED_CAST_LVALUE_class_##CLASS##_member_##MEMBER,__LINE__)*>(&INSTANCE)->MEMBER

#define BOOST_PROTECTED_CAST_RVALUE(CLASS, MEMBER, INSTANCE, LVALUE) \
	struct BOOST_PP_CAT(PROTECTED_CAST_RVALUE_class_##CLASS##_member_##MEMBER,__LINE__): public CLASS \
	{ using CLASS::MEMBER; }; \
	LVALUE static_cast<BOOST_PP_CAT(PROTECTED_CAST_RVALUE_class_##CLASS##_member_##MEMBER,__LINE__)*>(&INSTANCE)->MEMBER
