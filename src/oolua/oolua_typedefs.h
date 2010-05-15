///////////////////////////////////////////////////////////////////////////////
///  @file oolua_typedefs.h
///  Possible members for a Proxy_class typedefs
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////
#ifndef OOLUA_TYPEDEFS_H_
# define OOLUA_TYPEDEFS_H_

///  \addtogroup oolua_typedefs
///  @{

///  @struct OOLUA::Abstract
///  The class being mirrored is an abstract class.

///  @struct OOLUA::Less_op
///  Less than operator is defined for the type.

///  @struct OOLUA::Equal_op
///  Equal operator is defined for the type.

///  @struct OOLUA::Not_equal_op
///  Not equal operator is defined for the type.

///  @struct OOLUA::Less_equal_op
///  Less than or equal operator is defined for the type.

///  @struct OOLUA::Div_op
///  Division operator is defined for the type.

///  @struct OOLUA::Mul_op
///  Multiplication operator is defined for the type.

///  @struct OOLUA::Sub_op
///  Subtraction operator is defined for the type.

///  @struct OOLUA::Add_op
///  Addition operator is defined for the type.

///  @struct No_default_constructor
///  There is not a default constructor in the public interface
///  yet there are other constructors.

///  @struct No_public_constructors
///  There are no constructors in the public interface.

///  @struct OOLUA::No_public_destructor
///  There is not a destructor in the public interface
///  and OOLua will not attempt to delete an instance of this type

namespace OOLUA
{

	struct Abstract{};
	struct Less_op{};
	struct Equal_op{};
	struct Not_equal_op{};
	struct Less_equal_op{};
	struct Div_op{};
	struct Mul_op{};
	struct Sub_op{};
	struct Add_op{};
	struct No_default_constructor{};
	struct No_public_constructors{};
	struct No_public_destructor{};
}
	///  @}
#endif
