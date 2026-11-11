
// Nested macros are unfortunately needed when wanting to concatenate the
// *value* of a macro rather than the name of the macro itself. It also isn't
// possible to have a variable number of arguments on a macro so there needs
// to be a version for each number of arguments.
#define CONCAT2_PASTER(a,b) a ## b
#define CONCAT2(a,b) CONCAT2_PASTER(a,b)
#define CONCAT3_PASTER(a,b,c) a ## b ## c
#define CONCAT3(a,b,c) CONCAT3_PASTER(a,b,c)
#define CONCAT4_PASTER(a,b,c,d) a ## b ## c ## d
#define CONCAT4(a,b,c,d) CONCAT4_PASTER(a,b,c,d)
#define CONCAT5_PASTER(a,b,c,d,e) a ## b ## c ## d ## e
#define CONCAT5(a,b,c,d,e) CONCAT5_PASTER(a,b,c,d,e)
#define CONCAT6_PASTER(a,b,c,d,e,f) a ## b ## c ## d ## e ## f
#define CONCAT6(a,b,c,d,e,f) CONCAT5_PASTER(a,b,c,d,e,f)

// this is a convenient macro for debugging macros
// #pragma message(VAR_NAME_VALUE(<MACRO>
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "=" VALUE(var)

//
// SHADER_NAME = ND_<OPERATION>_<VARIANT>
// example shader name ND_clamp_float
//
// you MUST #define OPERATION in your shader to use this!
//
#define SHADER_NAME CONCAT4(ND_,OPERATION,_,VARIANT)

//
// Compare macros and unquoted constant strings with preprocessor
//  eg. STRING_CMP(VARIANT,vector2)
//
// The C preprocessor can't do string comparison directly, it can only
// compare numbers. To get around this limitation and provide a better user
// interface we convert the strings to numbers. This requires aprior knowledge
// of the strings to be compared.
//
// Each string that might be compared needs to have a macro defined as the
// string prefixed by "UNIQUE_ID_" and defined to a unique integer.
//
// A string which doesn't have a "UNIQUE_ID_" macro defined will be
// interpreted as 0 so 0 shouldn't be used for an id.
#define STRING_CMP(NAME1, NAME2) STRING_CMP_impl(NAME1, NAME2)
#define STRING_CMP_impl(NAME1, NAME2) UNIQUE_ID_ ## NAME1 == UNIQUE_ID_ ## NAME2

// Each string that might be compared needs to have a macro defined a the
// string prefixed by "UNIQUE_ID_" and defined to a unique integer.
// Using blocks of numbers for different uses will make it easier to
// keep them organized as things are added or deleted.
//
// A string which doesn't have a "UNIQUE_ID" macro defined will be
// interpreted as 0 so 0 shouldn't be used for an id.

// VARIANTS -- class variants for materialx maps
#define UNIQUE_ID_float      1
#define UNIQUE_ID_vector2    2
#define UNIQUE_ID_vector3    3
#define UNIQUE_ID_vector4    4
#define UNIQUE_ID_color3     5
#define UNIQUE_ID_color4     6
#define UNIQUE_ID_matrix33   7
#define UNIQUE_ID_matrix44   8
#define UNIQUE_ID_color3FA   9
#define UNIQUE_ID_color4FA   10
#define UNIQUE_ID_vector2FA  11
#define UNIQUE_ID_vector3FA  12
#define UNIQUE_ID_vector4FA  13
#define UNIQUE_ID_matrix33FA 14
#define UNIQUE_ID_matrix44FA 15
#define UNIQUE_ID_integer    16

#define UNIQUE_ID_float_color3    17
#define UNIQUE_ID_float_color4    18
#define UNIQUE_ID_float_vector2   19
#define UNIQUE_ID_float_vector3   20
#define UNIQUE_ID_float_vector4   21
#define UNIQUE_ID_color3_color4   22
#define UNIQUE_ID_color3_vector2  23
#define UNIQUE_ID_color3_vector3  24
#define UNIQUE_ID_color3_vector4  25
#define UNIQUE_ID_color4_color3   26
#define UNIQUE_ID_color4_vector2  27
#define UNIQUE_ID_color4_vector3  28
#define UNIQUE_ID_color4_vector4  29
#define UNIQUE_ID_vector2_color3  30
#define UNIQUE_ID_vector2_color4  31
#define UNIQUE_ID_vector2_vector3 32
#define UNIQUE_ID_vector2_vector4 33
#define UNIQUE_ID_vector3_color3  34
#define UNIQUE_ID_vector3_color4  35
#define UNIQUE_ID_vector3_vector2 36
#define UNIQUE_ID_vector3_vector4 37
#define UNIQUE_ID_vector4_color3  38
#define UNIQUE_ID_vector4_color4  39
#define UNIQUE_ID_vector4_vector2 40
#define UNIQUE_ID_vector4_vector3 41
#define UNIQUE_ID_boolean_float   42
#define UNIQUE_ID_boolean_color3  43
#define UNIQUE_ID_boolean_color4  44
#define UNIQUE_ID_boolean_vector2 45
#define UNIQUE_ID_boolean_vector3 46
#define UNIQUE_ID_boolean_vector4 47
#define UNIQUE_ID_boolean_integer 48
#define UNIQUE_ID_integer_float   49
#define UNIQUE_ID_integer_color3  50
#define UNIQUE_ID_integer_color4  51
#define UNIQUE_ID_integer_vector2 52
#define UNIQUE_ID_integer_vector3 53
#define UNIQUE_ID_integer_vector4 54
#define UNIQUE_ID_integer_boolean 55
#define UNIQUE_ID_vector3M4       56
#define UNIQUE_ID_vector2M3       57
#define UNIQUE_ID_floatB          58
#define UNIQUE_ID_color3B         59
#define UNIQUE_ID_color4B         60
#define UNIQUE_ID_vector2B        61
#define UNIQUE_ID_vector3B        62
#define UNIQUE_ID_vector4B        63
#define UNIQUE_ID_floatI          64
#define UNIQUE_ID_color3I         65
#define UNIQUE_ID_color4I         66
#define UNIQUE_ID_vector2I        67
#define UNIQUE_ID_vector3I        68
#define UNIQUE_ID_vector4I        69

// TYPES -- inputs usable by materialx maps
#define UNIQUE_ID_Bool  101
#define UNIQUE_ID_Int   102
#define UNIQUE_ID_Float 103
#define UNIQUE_ID_Vec2f 104
#define UNIQUE_ID_Vec3f 105
#define UNIQUE_ID_Vec4f 106
#define UNIQUE_ID_Color 107
#define UNIQUE_ID_Rgb   108
#define UNIQUE_ID_Rgba  109
#define UNIQUE_ID_Col4  110
#define UNIQUE_ID_Mat3f 111
#define UNIQUE_ID_Mat4f 112
