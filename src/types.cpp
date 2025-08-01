struct Ast;
struct Scope;
struct Entity;

enum BasicKind {
	Basic_Invalid,

	Basic_llvm_bool,
	Basic_bool,
	Basic_b8,
	Basic_b16,
	Basic_b32,
	Basic_b64,

	Basic_i8,
	Basic_u8,
	Basic_i16,
	Basic_u16,
	Basic_i32,
	Basic_u32,
	Basic_i64,
	Basic_u64,
	Basic_i128,
	Basic_u128,

	Basic_rune,

	Basic_f16,
	Basic_f32,
	Basic_f64,

	Basic_complex32,
	Basic_complex64,
	Basic_complex128,

	Basic_quaternion64,
	Basic_quaternion128,
	Basic_quaternion256,

	Basic_int,
	Basic_uint,
	Basic_uintptr,
	Basic_rawptr,
	Basic_string,  // ^u8 + int
	Basic_cstring, // ^u8
	Basic_any,     // rawptr + ^Type_Info

	Basic_typeid,

	// Endian Specific Types
	Basic_i16le,
	Basic_u16le,
	Basic_i32le,
	Basic_u32le,
	Basic_i64le,
	Basic_u64le,
	Basic_i128le,
	Basic_u128le,

	Basic_i16be,
	Basic_u16be,
	Basic_i32be,
	Basic_u32be,
	Basic_i64be,
	Basic_u64be,
	Basic_i128be,
	Basic_u128be,

	Basic_f16le,
	Basic_f32le,
	Basic_f64le,

	Basic_f16be,
	Basic_f32be,
	Basic_f64be,

	// Untyped types
	Basic_UntypedBool,
	Basic_UntypedInteger,
	Basic_UntypedFloat,
	Basic_UntypedComplex,
	Basic_UntypedQuaternion,
	Basic_UntypedString,
	Basic_UntypedRune,
	Basic_UntypedNil,
	Basic_UntypedUninit,

	Basic_COUNT,

	Basic_byte = Basic_u8,
};

enum BasicFlag {
	BasicFlag_Boolean     = GB_BIT(0),
	BasicFlag_Integer     = GB_BIT(1),
	BasicFlag_Unsigned    = GB_BIT(2),
	BasicFlag_Float       = GB_BIT(3),
	BasicFlag_Complex     = GB_BIT(4),
	BasicFlag_Quaternion  = GB_BIT(5),
	BasicFlag_Pointer     = GB_BIT(6),
	BasicFlag_String      = GB_BIT(7),
	BasicFlag_Rune        = GB_BIT(8),
	BasicFlag_Untyped     = GB_BIT(9),

	BasicFlag_LLVM        = GB_BIT(11),

	BasicFlag_EndianLittle = GB_BIT(13),
	BasicFlag_EndianBig    = GB_BIT(14),

	BasicFlag_Numeric        = BasicFlag_Integer | BasicFlag_Float   | BasicFlag_Complex | BasicFlag_Quaternion,
	BasicFlag_Ordered        = BasicFlag_Integer | BasicFlag_Float   | BasicFlag_String  | BasicFlag_Pointer | BasicFlag_Rune,
	BasicFlag_OrderedNumeric = BasicFlag_Integer | BasicFlag_Float   | BasicFlag_Rune,
	BasicFlag_ConstantType   = BasicFlag_Boolean | BasicFlag_Numeric | BasicFlag_String  | BasicFlag_Pointer | BasicFlag_Rune,
	BasicFlag_SimpleCompare  = BasicFlag_Boolean | BasicFlag_Integer | BasicFlag_Pointer | BasicFlag_Rune,
};

struct BasicType {
	BasicKind kind;
	u32       flags;
	i64       size; // -1 if arch. dep.
	String    name;
};

enum StructSoaKind : u8 {
	StructSoa_None    = 0,
	StructSoa_Fixed   = 1,
	StructSoa_Slice   = 2,
	StructSoa_Dynamic = 3,
};

struct TypeStruct {
	Slice<Entity *> fields;
	String *        tags;    // count == fields.count
	i64 *           offsets; // count == fields.count

	Ast *           node;
	Scope *         scope;

	i64             custom_align;
	i64             custom_min_field_align;
	i64             custom_max_field_align;
	Type *          polymorphic_params; // Type_Tuple
	Type *          polymorphic_parent;
	Wait_Signal     polymorphic_wait_signal;

	Type *          soa_elem;
	i32             soa_count;
	StructSoaKind   soa_kind;
	Wait_Signal     fields_wait_signal;
	BlockingMutex   soa_mutex;
	BlockingMutex   offset_mutex; // for settings offsets

	bool            is_polymorphic;
	bool            are_offsets_set             : 1;
	bool            are_offsets_being_processed : 1;
	bool            is_packed                   : 1;
	bool            is_raw_union                : 1;
	bool            is_poly_specialized         : 1;
};

struct TypeUnion {
	Slice<Type *> variants;

	Ast *         node;
	Scope *       scope;

	i64           variant_block_size;
	i64           custom_align;
	Type *        polymorphic_params; // Type_Tuple
	Type *        polymorphic_parent;
	Wait_Signal   polymorphic_wait_signal;

	i16           tag_size;
	bool          is_polymorphic;
	bool          is_poly_specialized;
	UnionTypeKind kind;
};

struct TypeProc {
	Ast *node;
	Scope *  scope;
	Type *   params;  // Type_Tuple
	Type *   results; // Type_Tuple
	i32      param_count;
	i32      result_count;
	isize    specialization_count;
	ProcCallingConvention calling_convention;
	i32      variadic_index;
	String   require_target_feature;
	String   enable_target_feature;
	// TODO(bill): Make this a flag set rather than bools
	bool     variadic;
	bool     require_results;
	bool     c_vararg;
	bool     is_polymorphic;
	bool     is_poly_specialized;
	bool     has_named_results;
	bool     diverging; // no return
	bool     return_by_pointer;
	bool     optional_ok;
};

#define TYPE_KINDS                                                \
	TYPE_KIND(Basic, BasicType)                               \
	TYPE_KIND(Named, struct {                                 \
		String  name;                                     \
		Type *  base;                                     \
		Entity *type_name; /* Entity_TypeName */          \
	})                                                        \
	TYPE_KIND(Generic, struct {                               \
		i64     id;                                       \
		String  name;                                     \
		Type *  specialized;                              \
		Scope * scope;                                    \
		Entity *entity;                                   \
	})                                                        \
	TYPE_KIND(Pointer, struct { Type *elem; })                \
	TYPE_KIND(MultiPointer, struct { Type *elem; })           \
	TYPE_KIND(Array,   struct {                               \
		Type *elem;                                       \
		i64   count;                                      \
		Type *generic_count;                              \
	})                                                        \
	TYPE_KIND(EnumeratedArray, struct {                       \
		Type *elem;                                       \
		Type *index;                                      \
		ExactValue *min_value;                            \
		ExactValue *max_value;                            \
		i64 count;                                        \
		TokenKind op;                                     \
		bool is_sparse;                                   \
	})                                                        \
	TYPE_KIND(Slice,   struct { Type *elem; })                \
	TYPE_KIND(DynamicArray, struct { Type *elem; })           \
	TYPE_KIND(Map, struct {                                   \
		Type *key;                                        \
		Type *value;                                      \
		Type *lookup_result_type;                         \
		Type *debug_metadata_type;                        \
	})                                                        \
	TYPE_KIND(Struct,  TypeStruct)                            \
	TYPE_KIND(Union,   TypeUnion)                             \
	TYPE_KIND(Enum, struct {                                  \
		Array<Entity *> fields;                           \
		Ast *node;                                        \
		Scope *  scope;                                   \
		Type *   base_type;                               \
		ExactValue *min_value;                            \
		ExactValue *max_value;                            \
		isize min_value_index;                            \
		isize max_value_index;                            \
	})                                                        \
	TYPE_KIND(Tuple, struct {                                 \
		Slice<Entity *> variables; /* Entity_Variable */  \
		i64 *           offsets;                          \
		BlockingMutex   mutex; /* for settings offsets */ \
		bool            are_offsets_being_processed;      \
		bool            are_offsets_set;                  \
		bool            is_packed;                        \
	})                                                        \
	TYPE_KIND(Proc, TypeProc)                                 \
	TYPE_KIND(BitSet, struct {                                \
		Type *elem;                                       \
		Type *underlying;                                 \
		i64   lower;                                      \
		i64   upper;                                      \
		Ast * node;                                       \
	})                                                        \
	TYPE_KIND(SimdVector, struct {                            \
		i64   count;                                      \
		Type *elem;                                       \
		Type *generic_count;                              \
	})                                                        \
	TYPE_KIND(Matrix, struct {                                \
		Type *elem;                                       \
		i64   row_count;                                  \
		i64   column_count;                               \
		Type *generic_row_count;                          \
		Type *generic_column_count;                       \
		i64   stride_in_bytes;                            \
		bool  is_row_major;                               \
	})                                                        \
	TYPE_KIND(BitField, struct {                              \
		Scope *         scope;                            \
		Type *          backing_type;                     \
		Slice<Entity *> fields;                           \
		String *        tags; /*count == fields.count*/   \
		Slice<u8>       bit_sizes;                        \
		Slice<i64>      bit_offsets;                      \
		Ast *           node;                             \
	})                                                        \
	TYPE_KIND(SoaPointer, struct { Type *elem; })


enum TypeKind {
	Type_Invalid,
#define TYPE_KIND(k, ...) GB_JOIN2(Type_, k),
	TYPE_KINDS
#undef TYPE_KIND
	Type_Count,
};

gb_global String const type_strings[] = {
	{cast(u8 *)"Invalid", gb_size_of("Invalid")},
#define TYPE_KIND(k, ...) {cast(u8 *)#k, gb_size_of(#k)-1},
	TYPE_KINDS
#undef TYPE_KIND
};

#define TYPE_KIND(k, ...) typedef __VA_ARGS__ GB_JOIN2(Type, k);
	TYPE_KINDS
#undef TYPE_KIND

enum TypeFlag : u32 {
	TypeFlag_Polymorphic     = 1<<1,
	TypeFlag_PolySpecialized = 1<<2,
	TypeFlag_InProcessOfCheckingPolymorphic = 1<<3,
};

struct Type {
	TypeKind kind;
	union {
#define TYPE_KIND(k, ...) GB_JOIN2(Type, k) k;
	TYPE_KINDS
#undef TYPE_KIND
	};

	// NOTE(bill): These need to be at the end to not affect the unionized data
	std::atomic<i64> cached_size;
	std::atomic<i64> cached_align;
	std::atomic<u32> flags; // TypeFlag
	bool failure;
};

// IMPORTANT NOTE(bill): This must match the same as the in core.odin
enum Typeid_Kind : u8 {
	Typeid_Invalid,
	Typeid_Integer,
	Typeid_Rune,
	Typeid_Float,
	Typeid_Complex,
	Typeid_Quaternion,
	Typeid_String,
	Typeid_Boolean,
	Typeid_Any,
	Typeid_Type_Id,
	Typeid_Pointer,
	Typeid_Multi_Pointer,
	Typeid_Procedure,
	Typeid_Array,
	Typeid_Enumerated_Array,
	Typeid_Dynamic_Array,
	Typeid_Slice,
	Typeid_Tuple,
	Typeid_Struct,
	Typeid_Union,
	Typeid_Enum,
	Typeid_Map,
	Typeid_Bit_Set,
	Typeid_Simd_Vector,
	Typeid_Matrix,
	Typeid_SoaPointer,
	Typeid_Bit_Field,

	Typeid__COUNT

};

// IMPORTANT NOTE(bill): This must match the same as the in core.odin
enum TypeInfoFlag : u32 {
	TypeInfoFlag_Comparable     = 1<<0,
	TypeInfoFlag_Simple_Compare = 1<<1,
};


enum : int {
	MATRIX_ELEMENT_COUNT_MIN = 1,
	MATRIX_ELEMENT_COUNT_MAX = 16,
	MATRIX_ELEMENT_MAX_SIZE = MATRIX_ELEMENT_COUNT_MAX * (2 * 8), // complex128

	SIMD_ELEMENT_COUNT_MIN = 1,
	SIMD_ELEMENT_COUNT_MAX = 64,
};


gb_internal bool is_type_comparable(Type *t);
gb_internal bool is_type_simple_compare(Type *t);
gb_internal Type *type_deref(Type *t, bool allow_multi_pointer=false);
gb_internal Type *base_type(Type *t);
gb_internal Type *alloc_type_multi_pointer(Type *elem);

gb_internal u32 type_info_flags_of_type(Type *type) {
	if (type == nullptr) {
		return 0;
	}
	u32 flags = 0;
	if (is_type_comparable(type)) {
		flags |= TypeInfoFlag_Comparable;
	}
	if (is_type_simple_compare(type)) {
		flags |= TypeInfoFlag_Comparable;
	}
	return flags;
}


// TODO(bill): Should I add extra information here specifying the kind of selection?
// e.g. field, constant, array field, type field, etc.
struct Selection {
	Entity *   entity;
	Array<i32> index;
	bool       indirect; // Set if there was a pointer deref anywhere down the line
	u8 swizzle_count;    // maximum components = 4
	u8 swizzle_indices;  // 2 bits per component, representing which swizzle index
	bool is_bit_field;
	bool pseudo_field;
};
gb_global Selection const empty_selection = {0};

gb_internal Selection make_selection(Entity *entity, Array<i32> index, bool indirect) {
	Selection s = {entity, index, indirect};
	return s;
}

gb_internal void selection_add_index(Selection *s, isize index) {
	// IMPORTANT NOTE(bill): this requires a stretchy buffer/dynamic array so it requires some form
	// of heap allocation
	// TODO(bill): Find a way to use a backing buffer for initial use as the general case is probably .count<3
	if (s->index.data == nullptr) {
		array_init(&s->index, heap_allocator());
	}
	array_add(&s->index, cast(i32)index);
}

gb_internal Selection selection_combine(Selection const &lhs, Selection const &rhs) {
	Selection new_sel = lhs;
	new_sel.indirect = lhs.indirect || rhs.indirect;
	new_sel.index = array_make<i32>(heap_allocator(), lhs.index.count+rhs.index.count);
	array_copy(&new_sel.index, lhs.index, 0);
	array_copy(&new_sel.index, rhs.index, lhs.index.count);
	return new_sel;
}

gb_internal Selection sub_selection(Selection const &sel, isize offset) {
	Selection res = {};
	res.index.data = sel.index.data + offset;
	res.index.count = gb_max(sel.index.count - offset, 0);
	res.index.capacity = res.index.count;
	return res;
}

gb_internal Selection trim_selection(Selection const &sel) {
	Selection res = {};
	res.index.data = sel.index.data;
	res.index.count = gb_max(sel.index.count - 1, 0);
	res.index.capacity = res.index.count;
	return res;
}


gb_global Type basic_types[] = {
	{Type_Basic, {Basic_Invalid,           0,                                          0, STR_LIT("invalid type")}},

	{Type_Basic, {Basic_llvm_bool,         BasicFlag_Boolean | BasicFlag_LLVM,         1, STR_LIT("llvm bool")}},

	{Type_Basic, {Basic_bool,              BasicFlag_Boolean,                          1, STR_LIT("bool")}},
	{Type_Basic, {Basic_b8,                BasicFlag_Boolean,                          1, STR_LIT("b8")}},
	{Type_Basic, {Basic_b16,               BasicFlag_Boolean,                          2, STR_LIT("b16")}},
	{Type_Basic, {Basic_b32,               BasicFlag_Boolean,                          4, STR_LIT("b32")}},
	{Type_Basic, {Basic_b64,               BasicFlag_Boolean,                          8, STR_LIT("b64")}},

	{Type_Basic, {Basic_i8,                BasicFlag_Integer,                          1, STR_LIT("i8")}},
	{Type_Basic, {Basic_u8,                BasicFlag_Integer | BasicFlag_Unsigned,     1, STR_LIT("u8")}},
	{Type_Basic, {Basic_i16,               BasicFlag_Integer,                          2, STR_LIT("i16")}},
	{Type_Basic, {Basic_u16,               BasicFlag_Integer | BasicFlag_Unsigned,     2, STR_LIT("u16")}},
	{Type_Basic, {Basic_i32,               BasicFlag_Integer,                          4, STR_LIT("i32")}},
	{Type_Basic, {Basic_u32,               BasicFlag_Integer | BasicFlag_Unsigned,     4, STR_LIT("u32")}},
	{Type_Basic, {Basic_i64,               BasicFlag_Integer,                          8, STR_LIT("i64")}},
	{Type_Basic, {Basic_u64,               BasicFlag_Integer | BasicFlag_Unsigned,     8, STR_LIT("u64")}},

	{Type_Basic, {Basic_i128,              BasicFlag_Integer,                         16, STR_LIT("i128")}},
	{Type_Basic, {Basic_u128,              BasicFlag_Integer | BasicFlag_Unsigned,    16, STR_LIT("u128")}},

	{Type_Basic, {Basic_rune,              BasicFlag_Integer | BasicFlag_Rune,         4, STR_LIT("rune")}},

	{Type_Basic, {Basic_f16,               BasicFlag_Float,                            2, STR_LIT("f16")}},
	{Type_Basic, {Basic_f32,               BasicFlag_Float,                            4, STR_LIT("f32")}},
	{Type_Basic, {Basic_f64,               BasicFlag_Float,                            8, STR_LIT("f64")}},

	{Type_Basic, {Basic_complex32,         BasicFlag_Complex,                          4, STR_LIT("complex32")}},
	{Type_Basic, {Basic_complex64,         BasicFlag_Complex,                          8, STR_LIT("complex64")}},
	{Type_Basic, {Basic_complex128,        BasicFlag_Complex,                         16, STR_LIT("complex128")}},

	{Type_Basic, {Basic_quaternion64,      BasicFlag_Quaternion,                       8, STR_LIT("quaternion64")}},
	{Type_Basic, {Basic_quaternion128,     BasicFlag_Quaternion,                      16, STR_LIT("quaternion128")}},
	{Type_Basic, {Basic_quaternion256,     BasicFlag_Quaternion,                      32, STR_LIT("quaternion256")}},

	{Type_Basic, {Basic_int,               BasicFlag_Integer,                         -1, STR_LIT("int")}},
	{Type_Basic, {Basic_uint,              BasicFlag_Integer | BasicFlag_Unsigned,    -1, STR_LIT("uint")}},
	{Type_Basic, {Basic_uintptr,           BasicFlag_Integer | BasicFlag_Unsigned,    -1, STR_LIT("uintptr")}},

	{Type_Basic, {Basic_rawptr,            BasicFlag_Pointer,                         -1, STR_LIT("rawptr")}},
	{Type_Basic, {Basic_string,            BasicFlag_String,                          -1, STR_LIT("string")}},
	{Type_Basic, {Basic_cstring,           BasicFlag_String,                          -1, STR_LIT("cstring")}},
	{Type_Basic, {Basic_any,               0,                                         16, STR_LIT("any")}},

	{Type_Basic, {Basic_typeid,            0,                                          8, STR_LIT("typeid")}},

	// Endian
	{Type_Basic, {Basic_i16le,  BasicFlag_Integer |                      BasicFlag_EndianLittle,  2, STR_LIT("i16le")}},
	{Type_Basic, {Basic_u16le,  BasicFlag_Integer | BasicFlag_Unsigned | BasicFlag_EndianLittle,  2, STR_LIT("u16le")}},
	{Type_Basic, {Basic_i32le,  BasicFlag_Integer |                      BasicFlag_EndianLittle,  4, STR_LIT("i32le")}},
	{Type_Basic, {Basic_u32le,  BasicFlag_Integer | BasicFlag_Unsigned | BasicFlag_EndianLittle,  4, STR_LIT("u32le")}},
	{Type_Basic, {Basic_i64le,  BasicFlag_Integer |                      BasicFlag_EndianLittle,  8, STR_LIT("i64le")}},
	{Type_Basic, {Basic_u64le,  BasicFlag_Integer | BasicFlag_Unsigned | BasicFlag_EndianLittle,  8, STR_LIT("u64le")}},
	{Type_Basic, {Basic_i128le, BasicFlag_Integer                      | BasicFlag_EndianLittle, 16, STR_LIT("i128le")}},
	{Type_Basic, {Basic_u128le, BasicFlag_Integer | BasicFlag_Unsigned | BasicFlag_EndianLittle, 16, STR_LIT("u128le")}},

	{Type_Basic, {Basic_i16be,  BasicFlag_Integer |                      BasicFlag_EndianBig,     2, STR_LIT("i16be")}},
	{Type_Basic, {Basic_u16be,  BasicFlag_Integer | BasicFlag_Unsigned | BasicFlag_EndianBig,     2, STR_LIT("u16be")}},
	{Type_Basic, {Basic_i32be,  BasicFlag_Integer |                      BasicFlag_EndianBig,     4, STR_LIT("i32be")}},
	{Type_Basic, {Basic_u32be,  BasicFlag_Integer | BasicFlag_Unsigned | BasicFlag_EndianBig,     4, STR_LIT("u32be")}},
	{Type_Basic, {Basic_i64be,  BasicFlag_Integer |                      BasicFlag_EndianBig,     8, STR_LIT("i64be")}},
	{Type_Basic, {Basic_u64be,  BasicFlag_Integer | BasicFlag_Unsigned | BasicFlag_EndianBig,     8, STR_LIT("u64be")}},
	{Type_Basic, {Basic_i128be, BasicFlag_Integer                      | BasicFlag_EndianBig,    16, STR_LIT("i128be")}},
	{Type_Basic, {Basic_u128be, BasicFlag_Integer | BasicFlag_Unsigned | BasicFlag_EndianBig,    16, STR_LIT("u128be")}},

	{Type_Basic, {Basic_f16le, BasicFlag_Float | BasicFlag_EndianLittle, 2, STR_LIT("f16le")}},
	{Type_Basic, {Basic_f32le, BasicFlag_Float | BasicFlag_EndianLittle, 4, STR_LIT("f32le")}},
	{Type_Basic, {Basic_f64le, BasicFlag_Float | BasicFlag_EndianLittle, 8, STR_LIT("f64le")}},

	{Type_Basic, {Basic_f16be, BasicFlag_Float | BasicFlag_EndianBig,    2, STR_LIT("f16be")}},
	{Type_Basic, {Basic_f32be, BasicFlag_Float | BasicFlag_EndianBig,    4, STR_LIT("f32be")}},
	{Type_Basic, {Basic_f64be, BasicFlag_Float | BasicFlag_EndianBig,    8, STR_LIT("f64be")}},

	// Untyped types
	{Type_Basic, {Basic_UntypedBool,       BasicFlag_Boolean    | BasicFlag_Untyped,   0, STR_LIT("untyped bool")}},
	{Type_Basic, {Basic_UntypedInteger,    BasicFlag_Integer    | BasicFlag_Untyped,   0, STR_LIT("untyped integer")}},
	{Type_Basic, {Basic_UntypedFloat,      BasicFlag_Float      | BasicFlag_Untyped,   0, STR_LIT("untyped float")}},
	{Type_Basic, {Basic_UntypedComplex,    BasicFlag_Complex    | BasicFlag_Untyped,   0, STR_LIT("untyped complex")}},
	{Type_Basic, {Basic_UntypedQuaternion, BasicFlag_Quaternion | BasicFlag_Untyped,   0, STR_LIT("untyped quaternion")}},
	{Type_Basic, {Basic_UntypedString,     BasicFlag_String     | BasicFlag_Untyped,   0, STR_LIT("untyped string")}},
	{Type_Basic, {Basic_UntypedRune,       BasicFlag_Integer    | BasicFlag_Untyped,   0, STR_LIT("untyped rune")}},
	{Type_Basic, {Basic_UntypedNil,        BasicFlag_Untyped,                          0, STR_LIT("untyped nil")}},
	{Type_Basic, {Basic_UntypedUninit,     BasicFlag_Untyped,                          0, STR_LIT("untyped uninitialized")}},
};

// gb_global Type basic_type_aliases[] = {
// 	// {Type_Basic, {Basic_byte, BasicFlag_Integer | BasicFlag_Unsigned, 1, STR_LIT("byte")}},
// 	// {Type_Basic, {Basic_rune, BasicFlag_Integer,                      4, STR_LIT("rune")}},
// };

gb_global Type *t_invalid         = &basic_types[Basic_Invalid];
gb_global Type *t_llvm_bool       = &basic_types[Basic_llvm_bool];
gb_global Type *t_bool            = &basic_types[Basic_bool];
gb_global Type *t_i8              = &basic_types[Basic_i8];
gb_global Type *t_u8              = &basic_types[Basic_u8];
gb_global Type *t_i16             = &basic_types[Basic_i16];
gb_global Type *t_u16             = &basic_types[Basic_u16];
gb_global Type *t_i32             = &basic_types[Basic_i32];
gb_global Type *t_u32             = &basic_types[Basic_u32];
gb_global Type *t_i64             = &basic_types[Basic_i64];
gb_global Type *t_u64             = &basic_types[Basic_u64];
gb_global Type *t_i128            = &basic_types[Basic_i128];
gb_global Type *t_u128            = &basic_types[Basic_u128];

gb_global Type *t_rune            = &basic_types[Basic_rune];

gb_global Type *t_f16             = &basic_types[Basic_f16];
gb_global Type *t_f32             = &basic_types[Basic_f32];
gb_global Type *t_f64             = &basic_types[Basic_f64];

gb_global Type *t_f16be           = &basic_types[Basic_f16be];
gb_global Type *t_f32be           = &basic_types[Basic_f32be];
gb_global Type *t_f64be           = &basic_types[Basic_f64be];

gb_global Type *t_f16le           = &basic_types[Basic_f16le];
gb_global Type *t_f32le           = &basic_types[Basic_f32le];
gb_global Type *t_f64le           = &basic_types[Basic_f64le];

gb_global Type *t_complex32       = &basic_types[Basic_complex32];
gb_global Type *t_complex64       = &basic_types[Basic_complex64];
gb_global Type *t_complex128      = &basic_types[Basic_complex128];

gb_global Type *t_quaternion64    = &basic_types[Basic_quaternion64];
gb_global Type *t_quaternion128   = &basic_types[Basic_quaternion128];
gb_global Type *t_quaternion256   = &basic_types[Basic_quaternion256];

gb_global Type *t_int             = &basic_types[Basic_int];
gb_global Type *t_uint            = &basic_types[Basic_uint];
gb_global Type *t_uintptr         = &basic_types[Basic_uintptr];

gb_global Type *t_rawptr          = &basic_types[Basic_rawptr];
gb_global Type *t_string          = &basic_types[Basic_string];
gb_global Type *t_cstring         = &basic_types[Basic_cstring];
gb_global Type *t_any             = &basic_types[Basic_any];

gb_global Type *t_typeid          = &basic_types[Basic_typeid];

gb_global Type *t_i16le           = &basic_types[Basic_i16le];
gb_global Type *t_u16le           = &basic_types[Basic_u16le];
gb_global Type *t_i32le           = &basic_types[Basic_i32le];
gb_global Type *t_u32le           = &basic_types[Basic_u32le];
gb_global Type *t_i64le           = &basic_types[Basic_i64le];
gb_global Type *t_u64le           = &basic_types[Basic_u64le];
gb_global Type *t_i128le          = &basic_types[Basic_i128le];
gb_global Type *t_u128le          = &basic_types[Basic_u128le];

gb_global Type *t_i16be           = &basic_types[Basic_i16be];
gb_global Type *t_u16be           = &basic_types[Basic_u16be];
gb_global Type *t_i32be           = &basic_types[Basic_i32be];
gb_global Type *t_u32be           = &basic_types[Basic_u32be];
gb_global Type *t_i64be           = &basic_types[Basic_i64be];
gb_global Type *t_u64be           = &basic_types[Basic_u64be];
gb_global Type *t_i128be          = &basic_types[Basic_i128be];
gb_global Type *t_u128be          = &basic_types[Basic_u128be];


gb_global Type *t_untyped_bool       = &basic_types[Basic_UntypedBool];
gb_global Type *t_untyped_integer    = &basic_types[Basic_UntypedInteger];
gb_global Type *t_untyped_float      = &basic_types[Basic_UntypedFloat];
gb_global Type *t_untyped_complex    = &basic_types[Basic_UntypedComplex];
gb_global Type *t_untyped_quaternion = &basic_types[Basic_UntypedQuaternion];
gb_global Type *t_untyped_string     = &basic_types[Basic_UntypedString];
gb_global Type *t_untyped_rune       = &basic_types[Basic_UntypedRune];
gb_global Type *t_untyped_nil        = &basic_types[Basic_UntypedNil];
gb_global Type *t_untyped_uninit     = &basic_types[Basic_UntypedUninit];



gb_global Type *t_u8_ptr       = nullptr;
gb_global Type *t_u8_multi_ptr = nullptr;
gb_global Type *t_int_ptr      = nullptr;
gb_global Type *t_i64_ptr      = nullptr;
gb_global Type *t_f64_ptr      = nullptr;
gb_global Type *t_u8_slice     = nullptr;
gb_global Type *t_string_slice = nullptr;


// Type generated for the "preload" file
gb_global Type *t_type_info                      = nullptr;
gb_global Type *t_type_info_enum_value           = nullptr;
gb_global Type *t_type_info_ptr                  = nullptr;
gb_global Type *t_type_info_enum_value_ptr       = nullptr;

gb_global Type *t_type_info_named                = nullptr;
gb_global Type *t_type_info_integer              = nullptr;
gb_global Type *t_type_info_rune                 = nullptr;
gb_global Type *t_type_info_float                = nullptr;
gb_global Type *t_type_info_complex              = nullptr;
gb_global Type *t_type_info_quaternion           = nullptr;
gb_global Type *t_type_info_any                  = nullptr;
gb_global Type *t_type_info_typeid               = nullptr;
gb_global Type *t_type_info_string               = nullptr;
gb_global Type *t_type_info_boolean              = nullptr;
gb_global Type *t_type_info_pointer              = nullptr;
gb_global Type *t_type_info_multi_pointer        = nullptr;
gb_global Type *t_type_info_procedure            = nullptr;
gb_global Type *t_type_info_array                = nullptr;
gb_global Type *t_type_info_enumerated_array     = nullptr;
gb_global Type *t_type_info_dynamic_array        = nullptr;
gb_global Type *t_type_info_slice                = nullptr;
gb_global Type *t_type_info_parameters           = nullptr;
gb_global Type *t_type_info_struct               = nullptr;
gb_global Type *t_type_info_union                = nullptr;
gb_global Type *t_type_info_enum                 = nullptr;
gb_global Type *t_type_info_map                  = nullptr;
gb_global Type *t_type_info_bit_set              = nullptr;
gb_global Type *t_type_info_simd_vector          = nullptr;
gb_global Type *t_type_info_matrix               = nullptr;
gb_global Type *t_type_info_soa_pointer          = nullptr;
gb_global Type *t_type_info_bit_field            = nullptr;

gb_global Type *t_type_info_named_ptr            = nullptr;
gb_global Type *t_type_info_integer_ptr          = nullptr;
gb_global Type *t_type_info_rune_ptr             = nullptr;
gb_global Type *t_type_info_float_ptr            = nullptr;
gb_global Type *t_type_info_complex_ptr          = nullptr;
gb_global Type *t_type_info_quaternion_ptr       = nullptr;
gb_global Type *t_type_info_any_ptr              = nullptr;
gb_global Type *t_type_info_typeid_ptr           = nullptr;
gb_global Type *t_type_info_string_ptr           = nullptr;
gb_global Type *t_type_info_boolean_ptr          = nullptr;
gb_global Type *t_type_info_pointer_ptr          = nullptr;
gb_global Type *t_type_info_multi_pointer_ptr    = nullptr;
gb_global Type *t_type_info_procedure_ptr        = nullptr;
gb_global Type *t_type_info_array_ptr            = nullptr;
gb_global Type *t_type_info_enumerated_array_ptr = nullptr;
gb_global Type *t_type_info_dynamic_array_ptr    = nullptr;
gb_global Type *t_type_info_slice_ptr            = nullptr;
gb_global Type *t_type_info_parameters_ptr       = nullptr;
gb_global Type *t_type_info_struct_ptr           = nullptr;
gb_global Type *t_type_info_union_ptr            = nullptr;
gb_global Type *t_type_info_enum_ptr             = nullptr;
gb_global Type *t_type_info_map_ptr              = nullptr;
gb_global Type *t_type_info_bit_set_ptr          = nullptr;
gb_global Type *t_type_info_simd_vector_ptr      = nullptr;
gb_global Type *t_type_info_matrix_ptr           = nullptr;
gb_global Type *t_type_info_soa_pointer_ptr      = nullptr;
gb_global Type *t_type_info_bit_field_ptr        = nullptr;

gb_global Type *t_allocator                      = nullptr;
gb_global Type *t_allocator_ptr                  = nullptr;
gb_global Type *t_context                        = nullptr;
gb_global Type *t_context_ptr                    = nullptr;
gb_global Type *t_allocator_error                = nullptr;

gb_global Type *t_source_code_location           = nullptr;
gb_global Type *t_source_code_location_ptr       = nullptr;

gb_global Type *t_load_directory_file            = nullptr;
gb_global Type *t_load_directory_file_ptr        = nullptr;
gb_global Type *t_load_directory_file_slice      = nullptr;

gb_global Type *t_map_info                       = nullptr;
gb_global Type *t_map_cell_info                  = nullptr;
gb_global Type *t_raw_map                        = nullptr;
gb_global Type *t_map_info_ptr                   = nullptr;
gb_global Type *t_map_cell_info_ptr              = nullptr;
gb_global Type *t_raw_map_ptr                    = nullptr;


gb_global Type *t_equal_proc  = nullptr;
gb_global Type *t_hasher_proc = nullptr;
gb_global Type *t_map_get_proc = nullptr;
gb_global Type *t_map_set_proc = nullptr;

gb_global Type *t_objc_object   = nullptr;
gb_global Type *t_objc_selector = nullptr;
gb_global Type *t_objc_class    = nullptr;
gb_global Type *t_objc_ivar     = nullptr;

gb_global Type *t_objc_id    = nullptr;
gb_global Type *t_objc_SEL   = nullptr;
gb_global Type *t_objc_Class = nullptr;
gb_global Type *t_objc_Ivar  = nullptr;

enum OdinAtomicMemoryOrder : i32 {
	OdinAtomicMemoryOrder_relaxed = 0, // unordered
	OdinAtomicMemoryOrder_consume = 1, // monotonic
	OdinAtomicMemoryOrder_acquire = 2,
	OdinAtomicMemoryOrder_release = 3,
	OdinAtomicMemoryOrder_acq_rel = 4,
	OdinAtomicMemoryOrder_seq_cst = 5,
	OdinAtomicMemoryOrder_COUNT,
};

char const *OdinAtomicMemoryOrder_strings[OdinAtomicMemoryOrder_COUNT] = {
	"Relaxed",
	"Consume",
	"Acquire",
	"Release",
	"Acq_Rel",
	"Seq_Cst",
};

gb_global Type *t_atomic_memory_order = nullptr;




gb_global RecursiveMutex g_type_mutex;

struct TypePath;

gb_internal i64      type_size_of   (Type *t);
gb_internal i64      type_align_of  (Type *t);
gb_internal i64      type_offset_of (Type *t, i64 index, Type **field_type_=nullptr);
gb_internal gbString type_to_string (Type *type, bool shorthand=true);
gb_internal gbString type_to_string (Type *type, gbAllocator allocator, bool shorthand=true);
gb_internal i64      type_size_of_internal(Type *t, TypePath *path);
gb_internal i64     type_align_of_internal(Type *t, TypePath *path);
gb_internal Type *   bit_set_to_int(Type *t);
gb_internal bool     are_types_identical(Type *x, Type *y);

gb_internal bool  is_type_pointer(Type *t);
gb_internal bool  is_type_multi_pointer(Type *t);
gb_internal bool  is_type_soa_pointer(Type *t);
gb_internal bool  is_type_proc(Type *t);
gb_internal bool  is_type_slice(Type *t);
gb_internal bool  is_type_integer(Type *t);
gb_internal bool  type_set_offsets(Type *t);


// IMPORTANT TODO(bill): SHould this TypePath code be removed since type cycle checking is handled much earlier on?

struct TypePath {
	RecursiveMutex mutex;
	Array<Entity *> path; // Entity_TypeName;
	bool failure;
};


gb_internal void type_path_init(TypePath *tp) {
	tp->path.allocator = heap_allocator();
}

gb_internal void type_path_free(TypePath *tp) {
	mutex_lock(&tp->mutex);
	array_free(&tp->path);
	mutex_unlock(&tp->mutex);
}

gb_internal void type_path_print_illegal_cycle(TypePath *tp, isize start_index) {
	GB_ASSERT(tp != nullptr);

	GB_ASSERT(start_index < tp->path.count);
	Entity *e = tp->path[start_index];
	GB_ASSERT(e != nullptr);
	error(e->token, "Illegal type declaration cycle of `%.*s`", LIT(e->token.string));
	// NOTE(bill): Print cycle, if it's deep enough
	for (isize j = start_index; j < tp->path.count; j++) {
		Entity *e = tp->path[j];
		error(e->token, "\t%.*s refers to", LIT(e->token.string));
	}
	// NOTE(bill): This will only print if the path count > 1
	error(e->token, "\t%.*s", LIT(e->token.string));
	tp->failure = true;
	e->type->failure = true;
	base_type(e->type)->failure = true;
}

gb_internal bool type_path_push(TypePath *tp, Type *t) {
	GB_ASSERT(tp != nullptr);
	if (t->kind != Type_Named) {
		return false;
	}
	Entity *e = t->Named.type_name;

	mutex_lock(&tp->mutex);

	for (isize i = 0; i < tp->path.count; i++) {
		Entity *p = tp->path[i];
		if (p == e) {
			type_path_print_illegal_cycle(tp, i);
		}
	}

	array_add(&tp->path, e);

	mutex_unlock(&tp->mutex);

	return true;
}

gb_internal void type_path_pop(TypePath *tp) {
	if (tp != nullptr) {
		mutex_lock(&tp->mutex);
		if (tp->path.count > 0) {
			array_pop(&tp->path);
		}
		mutex_unlock(&tp->mutex);
	}
}


#define FAILURE_SIZE      0
#define FAILURE_ALIGNMENT 0

gb_internal Type *base_type(Type *t) {
	for (;;) {
		if (t == nullptr) {
			break;
		}
		if (t->kind != Type_Named) {
			break;
		}
		if (t == t->Named.base) {
			return t_invalid;
		}
		t = t->Named.base;
	}
	return t;
}

gb_internal Type *base_named_type(Type *t) {
	if (t->kind != Type_Named) {
		return t_invalid;
	}

	Type *prev_named = t;
	t = t->Named.base;
	for (;;) {
		if (t == nullptr) {
			break;
		}
		if (t->kind != Type_Named) {
			break;
		}
		if (t == t->Named.base) {
			return t_invalid;
		}
		prev_named = t;
		t = t->Named.base;
	}
	return prev_named;
}

gb_internal Type *base_enum_type(Type *t) {
	Type *bt = base_type(t);
	if (bt != nullptr &&
	    bt->kind == Type_Enum) {
		return bt->Enum.base_type;
	}
	return t;
}

gb_internal Type *core_type(Type *t) {
	for (;;) {
		if (t == nullptr) {
			break;
		}

		switch (t->kind) {
		case Type_Named:
			if (t == t->Named.base) {
				return t_invalid;
			}
			t = t->Named.base;
			continue;
		case Type_Enum:
			t = t->Enum.base_type;
			continue;
		case Type_BitField:
			t = t->BitField.backing_type;
			continue;
		}
		break;
	}
	return t;
}

gb_internal void set_base_type(Type *t, Type *base) {
	if (t && t->kind == Type_Named) {
		t->Named.base = base;
	}
}


gb_internal Type *alloc_type(TypeKind kind) {
	// gbAllocator a = heap_allocator();
	gbAllocator a = permanent_allocator();
	Type *t = gb_alloc_item(a, Type);
	gb_zero_item(t);
	t->kind = kind;
	t->cached_size  = -1;
	t->cached_align = -1;
	return t;
}


gb_internal Type *alloc_type_generic(Scope *scope, i64 id, String name, Type *specialized) {
	Type *t = alloc_type(Type_Generic);
	t->Generic.id = id;
	t->Generic.name = name;
	t->Generic.specialized = specialized;
	t->Generic.scope = scope;
	return t;
}

gb_internal Type *alloc_type_pointer(Type *elem) {
	Type *t = alloc_type(Type_Pointer);
	t->Pointer.elem = elem;
	return t;
}

gb_internal Type *alloc_type_multi_pointer(Type *elem) {
	Type *t = alloc_type(Type_MultiPointer);
	t->MultiPointer.elem = elem;
	return t;
}

gb_internal Type *alloc_type_soa_pointer(Type *elem) {
	Type *t = alloc_type(Type_SoaPointer);
	t->SoaPointer.elem = elem;
	return t;
}

gb_internal Type *alloc_type_pointer_to_multi_pointer(Type *ptr) {
	Type *original_type = ptr;
	ptr = base_type(ptr);
	if (ptr->kind == Type_Pointer) {
		return alloc_type_multi_pointer(ptr->Pointer.elem);
	} else if (ptr->kind != Type_MultiPointer) {
		GB_PANIC("Invalid type: %s", type_to_string(original_type));
	}
	return original_type;
}

gb_internal Type *alloc_type_multi_pointer_to_pointer(Type *ptr) {
	Type *original_type = ptr;
	ptr = base_type(ptr);
	if (ptr->kind == Type_MultiPointer) {
		return alloc_type_pointer(ptr->MultiPointer.elem);
	} else if (ptr->kind != Type_Pointer) {
		GB_PANIC("Invalid type: %s", type_to_string(original_type));
	}
	return original_type;
}

gb_internal Type *alloc_type_array(Type *elem, i64 count, Type *generic_count = nullptr) {
	if (generic_count != nullptr) {
		Type *t = alloc_type(Type_Array);
		t->Array.elem = elem;
		t->Array.count = count;
		t->Array.generic_count = generic_count;
		return t;
	}
	Type *t = alloc_type(Type_Array);
	t->Array.elem = elem;
	t->Array.count = count;
	return t;
}

gb_internal Type *alloc_type_matrix(Type *elem, i64 row_count, i64 column_count, Type *generic_row_count, Type *generic_column_count, bool is_row_major) {
	if (generic_row_count != nullptr || generic_column_count != nullptr) {
		Type *t = alloc_type(Type_Matrix);
		t->Matrix.elem                 = elem;
		t->Matrix.row_count            = row_count;
		t->Matrix.column_count         = column_count;
		t->Matrix.generic_row_count    = generic_row_count;
		t->Matrix.generic_column_count = generic_column_count;
		t->Matrix.is_row_major         = is_row_major;
		return t;
	}
	Type *t = alloc_type(Type_Matrix);
	t->Matrix.elem = elem;
	t->Matrix.row_count = row_count;
	t->Matrix.column_count = column_count;
	t->Matrix.is_row_major = is_row_major;
	return t;
}


gb_internal Type *alloc_type_enumerated_array(Type *elem, Type *index, ExactValue const *min_value, ExactValue const *max_value, isize count, TokenKind op) {
	Type *t = alloc_type(Type_EnumeratedArray);
	t->EnumeratedArray.elem = elem;
	t->EnumeratedArray.index = index;
	t->EnumeratedArray.min_value = gb_alloc_item(permanent_allocator(), ExactValue);
	t->EnumeratedArray.max_value = gb_alloc_item(permanent_allocator(), ExactValue);
	gb_memmove(t->EnumeratedArray.min_value, min_value, gb_size_of(ExactValue));
	gb_memmove(t->EnumeratedArray.max_value, max_value, gb_size_of(ExactValue));
	t->EnumeratedArray.op = op;

	if (count == 0) {
		t->EnumeratedArray.count = 0;
	} else {
		t->EnumeratedArray.count = 1 + exact_value_to_i64(exact_value_sub(*max_value, *min_value));
	}
	return t;
}


gb_internal Type *alloc_type_slice(Type *elem) {
	Type *t = alloc_type(Type_Slice);
	t->Slice.elem = elem;
	return t;
}

gb_internal Type *alloc_type_dynamic_array(Type *elem) {
	Type *t = alloc_type(Type_DynamicArray);
	t->DynamicArray.elem = elem;
	return t;
}


gb_internal Type *alloc_type_struct() {
	Type *t = alloc_type(Type_Struct);
	return t;
}

gb_internal Type *alloc_type_struct_complete() {
	Type *t = alloc_type(Type_Struct);
	wait_signal_set(&t->Struct.fields_wait_signal);
	wait_signal_set(&t->Struct.polymorphic_wait_signal);
	return t;
}


gb_internal Type *alloc_type_union() {
	Type *t = alloc_type(Type_Union);
	return t;
}

gb_internal Type *alloc_type_enum() {
	Type *t = alloc_type(Type_Enum);
	t->Enum.min_value = gb_alloc_item(permanent_allocator(), ExactValue);
	t->Enum.max_value = gb_alloc_item(permanent_allocator(), ExactValue);
	return t;
}

gb_internal Type *alloc_type_bit_field() {
	Type *t = alloc_type(Type_BitField);
	return t;
}

gb_internal Type *alloc_type_named(String name, Type *base, Entity *type_name) {
	Type *t = alloc_type(Type_Named);
	t->Named.name = name;
	t->Named.base = base;
	if (base != t) {
		t->Named.base = base_type(base);
	}
	t->Named.type_name = type_name;
	return t;
}

gb_internal bool is_calling_convention_none(ProcCallingConvention calling_convention) {
	switch (calling_convention) {
	case ProcCC_None:
	case ProcCC_InlineAsm:
		return true;
	}
	return false;
}

gb_internal bool is_calling_convention_odin(ProcCallingConvention calling_convention) {
	switch (calling_convention) {
	case ProcCC_Odin:
	case ProcCC_Contextless:
		return true;
	}
	return false;
}

gb_internal Type *alloc_type_tuple() {
	Type *t = alloc_type(Type_Tuple);
	return t;
}

gb_internal Type *alloc_type_proc(Scope *scope, Type *params, isize param_count, Type *results, isize result_count, bool variadic, ProcCallingConvention calling_convention) {
	Type *t = alloc_type(Type_Proc);

	if (variadic) {
		if (param_count == 0) {
			GB_PANIC("variadic procedure must have at least one parameter");
		}
		GB_ASSERT(params != nullptr && params->kind == Type_Tuple);
		Entity *e = params->Tuple.variables[param_count-1];
		if (base_type(e->type)->kind != Type_Slice) {
			// NOTE(bill): For custom calling convention
			GB_PANIC("variadic parameter must be of type slice");
		}
	}

	t->Proc.scope        = scope;
	t->Proc.params       = params;
	t->Proc.param_count  = cast(i32)param_count;
	t->Proc.results      = results;
	t->Proc.result_count = cast(i32)result_count;
	t->Proc.variadic     = variadic;
	t->Proc.calling_convention = calling_convention;
	return t;
}

gb_internal bool is_type_valid_for_keys(Type *t);


gb_internal Type *alloc_type_bit_set() {
	Type *t = alloc_type(Type_BitSet);
	return t;
}



gb_internal Type *alloc_type_simd_vector(i64 count, Type *elem, Type *generic_count=nullptr) {
	Type *t = alloc_type(Type_SimdVector);
	t->SimdVector.count = count;
	t->SimdVector.elem = elem;
	t->SimdVector.generic_count = generic_count;
	return t;
}



////////////////////////////////////////////////////////////////


gb_internal Type *type_deref(Type *t, bool allow_multi_pointer) {
	if (t != nullptr) {
		Type *bt = base_type(t);
		if (bt == nullptr) {
			return nullptr;
		}
		switch (bt->kind) {
		case Type_Pointer:
			return bt->Pointer.elem;
		case Type_SoaPointer:
			{
				Type *elem = base_type(bt->SoaPointer.elem);
				GB_ASSERT(elem->kind == Type_Struct && elem->Struct.soa_kind != StructSoa_None);
				return elem->Struct.soa_elem;
			}
		case Type_MultiPointer:
			if (allow_multi_pointer) {
				return bt->MultiPointer.elem;
			}
			break;
		}
	}
	return t;
}

gb_internal bool is_type_named(Type *t) {
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return true;
	}
	return t->kind == Type_Named;
}

gb_internal bool is_type_boolean(Type *t) {
	// t = core_type(t);
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Boolean) != 0;
	}
	return false;
}
gb_internal bool is_type_integer(Type *t) {
	// t = core_type(t);
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Integer) != 0;
	}
	return false;
}
gb_internal bool is_type_integer_like(Type *t) {
	t = core_type(t);
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & (BasicFlag_Integer|BasicFlag_Boolean)) != 0;
	}
	if (t->kind == Type_BitSet) {
		if (t->BitSet.underlying) {
			return is_type_integer_like(t->BitSet.underlying);
		}
		return true;
	}
	return false;
}

gb_internal bool is_type_unsigned(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Unsigned) != 0;
	}
	if (t->kind == Type_Enum) {
		return (t->Enum.base_type->Basic.flags & BasicFlag_Unsigned) != 0;
	}
	return false;
}
gb_internal bool is_type_integer_128bit(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Integer) != 0 && t->Basic.size == 16;
	}
	return false;
}
gb_internal bool is_type_rune(Type *t) {
	// t = core_type(t);
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Rune) != 0;
	}
	return false;
}
gb_internal bool is_type_numeric(Type *t) {
	// t = core_type(t);
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Numeric) != 0;
	} else if (t->kind == Type_Enum) {
		return is_type_numeric(t->Enum.base_type);
	}
	// TODO(bill): Should this be here?
	if (t->kind == Type_Array) {
		return is_type_numeric(t->Array.elem);
	}
	return false;
}
gb_internal bool is_type_string(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_String) != 0;
	}
	return false;
}
gb_internal bool is_type_cstring(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return t->Basic.kind == Basic_cstring;
	}
	return false;
}
gb_internal bool is_type_typed(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Untyped) == 0;
	}
	return true;
}
gb_internal bool is_type_untyped(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Untyped) != 0;
	}
	return false;
}
gb_internal bool is_type_ordered(Type *t) {
	t = core_type(t);
	if (t == nullptr) { return false; }
	switch (t->kind) {
	case Type_Basic:
		return (t->Basic.flags & BasicFlag_Ordered) != 0;
	case Type_Pointer:
		return true;
	case Type_MultiPointer:
		return true;
	}
	return false;
}
gb_internal bool is_type_ordered_numeric(Type *t) {
	t = core_type(t);
	if (t == nullptr) { return false; }
	switch (t->kind) {
	case Type_Basic:
		return (t->Basic.flags & BasicFlag_OrderedNumeric) != 0;
	}
	return false;
}
gb_internal bool is_type_constant_type(Type *t) {
	t = core_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_ConstantType) != 0;
	}
	if (t->kind == Type_BitSet) {
		return true;
	}
	if (t->kind == Type_Proc) {
		return true;
	}
	return false;
}
gb_internal bool is_type_float(Type *t) {
	t = core_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Float) != 0;
	}
	return false;
}
gb_internal bool is_type_complex(Type *t) {
	t = core_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Complex) != 0;
	}
	return false;
}
gb_internal bool is_type_quaternion(Type *t) {
	t = core_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Quaternion) != 0;
	}
	return false;
}
gb_internal bool is_type_complex_or_quaternion(Type *t) {
	t = core_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & (BasicFlag_Complex|BasicFlag_Quaternion)) != 0;
	}
	return false;
}
gb_internal bool is_type_pointer(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & BasicFlag_Pointer) != 0;
	}
	return t->kind == Type_Pointer;
}
gb_internal bool is_type_soa_pointer(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_SoaPointer;
}
gb_internal bool is_type_multi_pointer(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_MultiPointer;
}
gb_internal bool is_type_internally_pointer_like(Type *t) {
	return is_type_pointer(t) || is_type_multi_pointer(t) || is_type_cstring(t) || is_type_proc(t);
}

gb_internal bool is_type_tuple(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_Tuple;
}
gb_internal bool is_type_uintptr(Type *t) {
	if (t->kind == Type_Basic) {
		return (t->Basic.kind == Basic_uintptr);
	}
	return false;
}
gb_internal bool is_type_rawptr(Type *t) {
	if (t->kind == Type_Basic) {
		return t->Basic.kind == Basic_rawptr;
	}
	return false;
}
gb_internal bool is_type_u8(Type *t) {
	if (t->kind == Type_Basic) {
		return t->Basic.kind == Basic_u8;
	}
	return false;
}
gb_internal bool is_type_array(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_Array;
}
gb_internal bool is_type_enumerated_array(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_EnumeratedArray;
}
gb_internal bool is_type_matrix(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_Matrix;
}

gb_internal i64 matrix_align_of(Type *t, struct TypePath *tp) {
	t = base_type(t);
	GB_ASSERT(t->kind == Type_Matrix);

	Type *elem = t->Matrix.elem;
	i64 row_count = gb_max(t->Matrix.row_count, 1);
	i64 column_count = gb_max(t->Matrix.column_count, 1);

	bool pop = type_path_push(tp, elem);
	if (tp->failure) {
		return FAILURE_ALIGNMENT;
	}

	i64 elem_align = type_align_of_internal(elem, tp);
	if (pop) type_path_pop(tp);

	i64 elem_size = type_size_of(elem);


	// NOTE(bill, 2021-10-25): The alignment strategy here is to have zero padding
	// It would be better for performance to pad each column so that each column
	// could be maximally aligned but as a compromise, having no padding will be
	// beneficial to third libraries that assume no padding

	i64 total_expected_size = row_count*column_count*elem_size;
	// i64 min_alignment = prev_pow2(elem_align * row_count);
	i64 min_alignment = prev_pow2(total_expected_size);
	while (total_expected_size != 0 && (total_expected_size % min_alignment) != 0) {
		min_alignment >>= 1;
	}
	min_alignment = gb_max(min_alignment, elem_align);

	i64 align = gb_min(min_alignment, build_context.max_simd_align);
	return align;
}


gb_internal i64 matrix_type_stride_in_bytes(Type *t, struct TypePath *tp) {
	t = base_type(t);
	GB_ASSERT(t->kind == Type_Matrix);
	if (t->Matrix.stride_in_bytes != 0) {
		return t->Matrix.stride_in_bytes;
	} else if (t->Matrix.row_count == 0) {
		return 0;
	}

	i64 elem_size;
	if (tp != nullptr) {
		elem_size = type_size_of_internal(t->Matrix.elem, tp);
	} else {
		elem_size = type_size_of(t->Matrix.elem);
	}

	i64 stride_in_bytes = 0;

	// NOTE(bill, 2021-10-25): The alignment strategy here is to have zero padding
	// It would be better for performance to pad each column/row so that each column/row
	// could be maximally aligned but as a compromise, having no padding will be
	// beneficial to third libraries that assume no padding

	if (t->Matrix.is_row_major) {
		stride_in_bytes = elem_size*t->Matrix.column_count;
	} else {
		stride_in_bytes = elem_size*t->Matrix.row_count;
	}
	t->Matrix.stride_in_bytes = stride_in_bytes;
	return stride_in_bytes;
}

gb_internal i64 matrix_type_stride_in_elems(Type *t) {
	t = base_type(t);
	GB_ASSERT(t->kind == Type_Matrix);
	i64 stride = matrix_type_stride_in_bytes(t, nullptr);
	return stride/gb_max(1, type_size_of(t->Matrix.elem));
}


gb_internal i64 matrix_type_total_internal_elems(Type *t) {
	t = base_type(t);
	GB_ASSERT(t->kind == Type_Matrix);
	i64 size = type_size_of(t);
	i64 elem_size = type_size_of(t->Matrix.elem);
	return size/gb_max(elem_size, 1);
}

gb_internal i64 matrix_indices_to_offset(Type *t, i64 row_index, i64 column_index) {
	t = base_type(t);
	GB_ASSERT(t->kind == Type_Matrix);
	GB_ASSERT(0 <= row_index && row_index < t->Matrix.row_count);
	GB_ASSERT(0 <= column_index && column_index < t->Matrix.column_count);
	i64 stride_elems = matrix_type_stride_in_elems(t);
	if (t->Matrix.is_row_major) {
		return column_index + stride_elems*row_index;
	} else {
		// NOTE(bill): Column-major layout internally
		return row_index + stride_elems*column_index;
	}
}

gb_internal i64 matrix_row_major_index_to_offset(Type *t, i64 index) {
	t = base_type(t);
	GB_ASSERT(t->kind == Type_Matrix);

	i64 row_index    = index/t->Matrix.column_count;
	i64 column_index = index%t->Matrix.column_count;
	return matrix_indices_to_offset(t, row_index, column_index);
}
gb_internal i64 matrix_column_major_index_to_offset(Type *t, i64 index) {
	t = base_type(t);
	GB_ASSERT(t->kind == Type_Matrix);

	i64 row_index    = index%t->Matrix.row_count;
	i64 column_index = index/t->Matrix.row_count;
	return matrix_indices_to_offset(t, row_index, column_index);
}


gb_internal bool is_matrix_square(Type *t) {
	t = base_type(t);
	GB_ASSERT(t->kind == Type_Matrix);
	return t->Matrix.row_count == t->Matrix.column_count;
}

gb_internal bool is_type_valid_for_matrix_elems(Type *t) {
	t = base_type(t);
	if (is_type_integer(t)) {
		return true;
	} else if (is_type_float(t)) {
		return true;
	} else if (is_type_complex(t)) {
		return true;
	}
	if (t->kind == Type_Generic) {
		return true;
	}
	return false;
}

gb_internal bool is_type_dynamic_array(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_DynamicArray;
}
gb_internal bool is_type_slice(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_Slice;
}
gb_internal bool is_type_proc(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_Proc;
}
gb_internal bool is_type_asm_proc(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_Proc && t->Proc.calling_convention == ProcCC_InlineAsm;
}
gb_internal bool is_type_simd_vector(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_SimdVector;
}

gb_internal Type *base_array_type(Type *t) {
	Type *bt = base_type(t);
	if (is_type_array(bt)) {
		return bt->Array.elem;
	} else if (is_type_enumerated_array(bt)) {
		return bt->EnumeratedArray.elem;
	} else if (is_type_simd_vector(bt)) {
		return bt->SimdVector.elem;
	} else if (is_type_matrix(bt)) {
		return bt->Matrix.elem;
	}
	return t;
}


gb_internal Type *base_any_array_type(Type *t) {
	Type *bt = base_type(t);
	if (is_type_array(bt)) {
		return bt->Array.elem;
	} else if (is_type_slice(bt)) {
		return bt->Slice.elem;
	} else if (is_type_dynamic_array(bt)) {
		return bt->DynamicArray.elem;
	} else if (is_type_enumerated_array(bt)) {
		return bt->EnumeratedArray.elem;
	} else if (is_type_simd_vector(bt)) {
		return bt->SimdVector.elem;
	} else if (is_type_matrix(bt)) {
		return bt->Matrix.elem;
	}
	return t;
}


gb_internal bool is_type_generic(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	return t->kind == Type_Generic;
}

gb_internal bool is_type_u8_slice(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Slice) {
		return is_type_u8(t->Slice.elem);
	}
	return false;
}
gb_internal bool is_type_u8_array(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Array) {
		return is_type_u8(t->Array.elem);
	}
	return false;
}
gb_internal bool is_type_u8_ptr(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Pointer) {
		return is_type_u8(t->Slice.elem);
	}
	return false;
}
gb_internal bool is_type_u8_multi_ptr(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_MultiPointer) {
		return is_type_u8(t->Slice.elem);
	}
	return false;
}
gb_internal bool is_type_rune_array(Type *t) {
	t = base_type(t);
	if (t == nullptr) { return false; }
	if (t->kind == Type_Array) {
		return is_type_rune(t->Array.elem);
	}
	return false;
}


gb_internal bool is_type_array_like(Type *t) {
	return is_type_array(t) || is_type_enumerated_array(t);
}
gb_internal i64 get_array_type_count(Type *t) {
	Type *bt = base_type(t);
	if (bt->kind == Type_Array) {
		return bt->Array.count;
	} else if (bt->kind == Type_EnumeratedArray) {
		return bt->EnumeratedArray.count;
	} else if (bt->kind == Type_SimdVector) {
		return bt->SimdVector.count;
	}
	GB_ASSERT(is_type_array_like(t));
	return -1;
}



gb_internal Type *core_array_type(Type *t) {
	for (;;) {
		t = base_array_type(t);
		switch (t->kind) {
		case Type_Array:
		case Type_EnumeratedArray:
		case Type_SimdVector:
		case Type_Matrix:
			break;
		default:
			return t;
		}
	}
}

gb_internal i32 type_math_rank(Type *t) {
	i32 rank = 0;
	for (;;) {
		t = base_type(t);
		switch (t->kind) {
		case Type_Array:
			rank += 1;
			t = t->Array.elem;
			break;
		case Type_Matrix:
			rank += 2;
			t = t->Matrix.elem;
			break;
		default:
			return rank;
		}
	}
}


gb_internal Type *base_complex_elem_type(Type *t) {
	t = core_type(t);
	if (t->kind == Type_Basic) {
		switch (t->Basic.kind) {
		case Basic_complex32:         return t_f16;
		case Basic_complex64:         return t_f32;
		case Basic_complex128:        return t_f64;
		case Basic_quaternion64:      return t_f16;
		case Basic_quaternion128:     return t_f32;
		case Basic_quaternion256:     return t_f64;
		case Basic_UntypedComplex:    return t_untyped_float;
		case Basic_UntypedQuaternion: return t_untyped_float;
		}
	}
	GB_PANIC("Invalid complex type");
	return t_invalid;
}

gb_internal bool is_type_struct(Type *t) {
	t = base_type(t);
	return t->kind == Type_Struct;
}
gb_internal bool is_type_union(Type *t) {
	t = base_type(t);
	return t->kind == Type_Union;
}
gb_internal bool is_type_soa_struct(Type *t) {
	t = base_type(t);
	return t->kind == Type_Struct && t->Struct.soa_kind != StructSoa_None;
}

gb_internal bool is_type_raw_union(Type *t) {
	t = base_type(t);
	return (t->kind == Type_Struct && t->Struct.is_raw_union);
}
gb_internal bool is_type_enum(Type *t) {
	t = base_type(t);
	return (t->kind == Type_Enum);
}
gb_internal bool is_type_bit_set(Type *t) {
	t = base_type(t);
	return (t->kind == Type_BitSet);
}
gb_internal bool is_type_bit_field(Type *t) {
	t = base_type(t);
	return (t->kind == Type_BitField);
}
gb_internal bool is_type_map(Type *t) {
	t = base_type(t);
	return t->kind == Type_Map;
}

gb_internal bool is_type_union_maybe_pointer(Type *t) {
	t = base_type(t);
	if (t->kind == Type_Union && t->Union.variants.count == 1) {
		Type *v = t->Union.variants[0];
		return is_type_internally_pointer_like(v);
	}
	return false;
}


gb_internal bool is_type_union_maybe_pointer_original_alignment(Type *t) {
	t = base_type(t);
	if (t->kind == Type_Union && t->Union.variants.count == 1) {
		Type *v = t->Union.variants[0];
		if (is_type_internally_pointer_like(v)) {
			return type_align_of(v) == type_align_of(t);
		}
	}
	return false;
}


enum TypeEndianKind {
	TypeEndian_Platform,
	TypeEndian_Little,
	TypeEndian_Big,
};

gb_internal TypeEndianKind type_endian_kind_of(Type *t) {
	t = core_type(t);
	if (t->kind == Type_Basic) {
		if (t->Basic.flags & BasicFlag_EndianLittle) {
			return TypeEndian_Little;
		}
		if (t->Basic.flags & BasicFlag_EndianBig) {
			return TypeEndian_Big;
		}
	} else if (t->kind == Type_BitSet) {
		return type_endian_kind_of(bit_set_to_int(t));
	}
	return TypeEndian_Platform;
}


gb_internal bool is_type_endian_big(Type *t) {
	t = core_type(t);
	if (t->kind == Type_Basic) {
		if (t->Basic.flags & BasicFlag_EndianBig) {
			return true;
		} else if (t->Basic.flags & BasicFlag_EndianLittle) {
			return false;
		}
		return build_context.endian_kind == TargetEndian_Big;
	} else if (t->kind == Type_BitSet) {
		return is_type_endian_big(bit_set_to_int(t));
	} else if (t->kind == Type_Pointer) {
		return is_type_endian_big(&basic_types[Basic_uintptr]);
	}
	return build_context.endian_kind == TargetEndian_Big;
}
gb_internal bool is_type_endian_little(Type *t) {
	t = core_type(t);
	if (t->kind == Type_Basic) {
		if (t->Basic.flags & BasicFlag_EndianLittle) {
			return true;
		} else if (t->Basic.flags & BasicFlag_EndianBig) {
			return false;
		}
		return build_context.endian_kind == TargetEndian_Little;
	} else if (t->kind == Type_BitSet) {
		return is_type_endian_little(bit_set_to_int(t));
	} else if (t->kind == Type_Pointer) {
		return is_type_endian_little(&basic_types[Basic_uintptr]);
	}
	return build_context.endian_kind == TargetEndian_Little;
}

gb_internal bool is_type_endian_platform(Type *t) {
	t = core_type(t);
	if (t->kind == Type_Basic) {
		return (t->Basic.flags & (BasicFlag_EndianLittle|BasicFlag_EndianBig)) == 0;
	} else if (t->kind == Type_BitSet) {
		return is_type_endian_platform(bit_set_to_int(t));
	} else if (t->kind == Type_Pointer) {
		return is_type_endian_platform(&basic_types[Basic_uintptr]);
	}
	return false;
}

gb_internal bool types_have_same_internal_endian(Type *a, Type *b) {
	return is_type_endian_little(a) == is_type_endian_little(b);
}
gb_internal bool is_type_endian_specific(Type *t) {
	t = core_type(t);
	if (t->kind == Type_BitSet) {
		t = bit_set_to_int(t);
	}
	if (t->kind == Type_Basic) {
		switch (t->Basic.kind) {
		case Basic_i16le:
		case Basic_u16le:
		case Basic_i32le:
		case Basic_u32le:
		case Basic_i64le:
		case Basic_u64le:
		case Basic_u128le:
			return true;

		case Basic_i16be:
		case Basic_u16be:
		case Basic_i32be:
		case Basic_u32be:
		case Basic_i64be:
		case Basic_u64be:
		case Basic_u128be:
			return true;

		case Basic_f16le:
		case Basic_f16be:
		case Basic_f32le:
		case Basic_f32be:
		case Basic_f64le:
		case Basic_f64be:
			return true;
		}
	}

	return false;
}

gb_internal bool is_type_dereferenceable(Type *t) {
	if (is_type_rawptr(t)) {
		return false;
	}
	return is_type_pointer(t) || is_type_soa_pointer(t);
}



gb_internal bool is_type_different_to_arch_endianness(Type *t) {
	switch (build_context.endian_kind) {
	case TargetEndian_Little:
		return !is_type_endian_little(t);
	case TargetEndian_Big:
		return !is_type_endian_big(t);
	}
	return false;
}

gb_internal Type *integer_endian_type_to_platform_type(Type *t) {
	t = core_type(t);
	if (t->kind == Type_BitSet) {
		t = bit_set_to_int(t);
	}
	GB_ASSERT_MSG(t->kind == Type_Basic, "%s", type_to_string(t));

	switch (t->Basic.kind) {
	// Endian Specific Types
	case Basic_i16le: return t_i16;
	case Basic_u16le: return t_u16;
	case Basic_i32le: return t_i32;
	case Basic_u32le: return t_u32;
	case Basic_i64le: return t_i64;
	case Basic_u64le: return t_u64;
	case Basic_i128le: return t_i128;
	case Basic_u128le: return t_u128;

	case Basic_i16be: return t_i16;
	case Basic_u16be: return t_u16;
	case Basic_i32be: return t_i32;
	case Basic_u32be: return t_u32;
	case Basic_i64be: return t_i64;
	case Basic_u64be: return t_u64;
	case Basic_i128be: return t_i128;
	case Basic_u128be: return t_u128;

	case Basic_f16le: return t_f16;
	case Basic_f16be: return t_f16;
	case Basic_f32le: return t_f32;
	case Basic_f32be: return t_f32;
	case Basic_f64le: return t_f64;
	case Basic_f64be: return t_f64;
	}

	return t;
}



gb_internal bool is_type_any(Type *t) {
	t = base_type(t);
	return (t->kind == Type_Basic && t->Basic.kind == Basic_any);
}
gb_internal bool is_type_typeid(Type *t) {
	t = base_type(t);
	return (t->kind == Type_Basic && t->Basic.kind == Basic_typeid);
}
gb_internal bool is_type_untyped_nil(Type *t) {
	t = base_type(t);
	// NOTE(bill): checking for `nil` or `---` at once is just to improve the error handling
	return (t->kind == Type_Basic && (t->Basic.kind == Basic_UntypedNil || t->Basic.kind == Basic_UntypedUninit));
}
gb_internal bool is_type_untyped_uninit(Type *t) {
	t = base_type(t);
	// NOTE(bill): checking for `nil` or `---` at once is just to improve the error handling
	return (t->kind == Type_Basic && t->Basic.kind == Basic_UntypedUninit);
}

gb_internal bool is_type_empty_union(Type *t) {
	if (t == nullptr) {
		return false;
	}
	t = base_type(t);
	if (t == nullptr) {
		return false;
	}
	return t->kind == Type_Union && t->Union.variants.count == 0;
}

gb_internal bool is_type_valid_for_keys(Type *t) {
	t = core_type(t);
	if (t->kind == Type_Generic) {
		return true;
	}
	if (is_type_untyped(t)) {
		return false;
	}
	return type_size_of(t) > 0 && is_type_comparable(t);
}

gb_internal bool is_type_valid_bit_set_elem(Type *t) {
	if (is_type_enum(t)) {
		return true;
	}
	t = core_type(t);
	if (t->kind == Type_Generic) {
		return true;
	}
	return false;
}


gb_internal bool is_valid_bit_field_backing_type(Type *type) {
	if (type == nullptr) {
		return false;
	}
	type = base_type(type);
	if (is_type_untyped(type)) {
		return false;
	}
	if (is_type_integer(type)) {
		return true;
	}
	if (type->kind == Type_Array) {
		return is_type_integer(type->Array.elem);
	}
	return false;
}

gb_internal Type *bit_set_to_int(Type *t) {
	GB_ASSERT(is_type_bit_set(t));
	Type *bt = base_type(t);
	Type *underlying = bt->BitSet.underlying;
	if (underlying != nullptr && is_type_integer(underlying)) {
		return underlying;
	}
	if (underlying != nullptr && is_valid_bit_field_backing_type(underlying)) {
		return underlying;
	}

	i64 sz = type_size_of(t);
	switch (sz) {
	case 0:  return t_u8;
	case 1:  return t_u8;
	case 2:  return t_u16;
	case 4:  return t_u32;
	case 8:  return t_u64;
	case 16: return t_u128;
	}
	GB_PANIC("Unknown bit_set size");
	return nullptr;
}

gb_internal bool is_type_valid_vector_elem(Type *t) {
	t = base_type(t);
	if (t->kind == Type_Basic) {
		if (t->Basic.flags & BasicFlag_EndianLittle) {
			return false;
		}
		if (t->Basic.flags & BasicFlag_EndianBig) {
			return false;
		}
		if (is_type_integer(t)) {
			return !is_type_integer_128bit(t);
		}
		if (is_type_float(t)) {
			return true;
		}
		if (is_type_boolean(t)) {
			return true;
		}
		if (t->Basic.kind == Basic_rawptr) {
			return true;
		}
	}
	return false;
}


gb_internal bool is_type_indexable(Type *t) {
	Type *bt = base_type(t);
	switch (bt->kind) {
	case Type_Basic:
		return bt->Basic.kind == Basic_string;
	case Type_Array:
	case Type_Slice:
	case Type_DynamicArray:
	case Type_Map:
		return true;
	case Type_MultiPointer:
		return true;
	case Type_EnumeratedArray:
		return true;
	case Type_Matrix:
		return true;
	}
	return false;
}

gb_internal bool is_type_sliceable(Type *t) {
	Type *bt = base_type(t);
	switch (bt->kind) {
	case Type_Basic:
		return bt->Basic.kind == Basic_string;
	case Type_Array:
	case Type_Slice:
	case Type_DynamicArray:
		return true;
	case Type_EnumeratedArray:
		return false;
	case Type_Matrix:
		return false;
	}
	return false;
}

gb_internal Entity *type_get_polymorphic_parent(Type *t, Type **params_) {
	t = base_type(t);
	if (t == nullptr) {
		return nullptr;
	}
	Type *parent = nullptr;
	if (t->kind == Type_Struct) {
		parent = t->Struct.polymorphic_parent;
		if (params_) *params_ = t->Struct.polymorphic_params;
	} else if (t->kind == Type_Union) {
		parent = t->Union.polymorphic_parent;
		if (params_) *params_ = t->Union.polymorphic_params;
	}
	if (parent != nullptr) {
		GB_ASSERT(parent->kind == Type_Named);

		return parent->Named.type_name;
	}
	return nullptr;
}

gb_internal bool is_type_polymorphic_record(Type *t) {
	t = base_type(t);
	if (t->kind == Type_Struct) {
		return t->Struct.is_polymorphic;
	} else if (t->kind == Type_Union) {
		return t->Union.is_polymorphic;
	}
	return false;
}

gb_internal Scope *polymorphic_record_parent_scope(Type *t) {
	t = base_type(t);
	if (is_type_polymorphic_record(t)) {
		if (t->kind == Type_Struct) {
			return t->Struct.scope->parent;
		} else if (t->kind == Type_Union) {
			return t->Union.scope->parent;
		}
	}
	return nullptr;
}

gb_internal bool is_type_polymorphic_record_specialized(Type *t) {
	t = base_type(t);
	if (t->kind == Type_Struct) {
		return t->Struct.is_poly_specialized;
	} else if (t->kind == Type_Union) {
		return t->Union.is_poly_specialized;
	}
	return false;
}

gb_internal bool is_type_polymorphic_record_unspecialized(Type *t) {
	t = base_type(t);
	if (t->kind == Type_Struct) {
		return t->Struct.is_polymorphic && !t->Struct.is_poly_specialized;
	} else if (t->kind == Type_Union) {
		return t->Union.is_polymorphic && !t->Union.is_poly_specialized;
	}
	return false;
}


gb_internal TypeTuple *get_record_polymorphic_params(Type *t) {
	t = base_type(t);
	switch (t->kind) {
	case Type_Struct:
		wait_signal_until_available(&t->Struct.polymorphic_wait_signal);
		if (t->Struct.polymorphic_params) {
			return &t->Struct.polymorphic_params->Tuple;
		}
		break;
	case Type_Union:
		wait_signal_until_available(&t->Union.polymorphic_wait_signal);
		if (t->Union.polymorphic_params) {
			return &t->Union.polymorphic_params->Tuple;
		}
		break;
	}
	return nullptr;
}


gb_internal bool is_type_polymorphic(Type *t, bool or_specialized=false) {
	if (t == nullptr) {
		return false;
	}
	if (t->flags & TypeFlag_InProcessOfCheckingPolymorphic) {
		return false;
	}

	switch (t->kind) {
	case Type_Generic:
		return true;

	case Type_Named:
		{
			u32 flags = t->flags;
			t->flags |= TypeFlag_InProcessOfCheckingPolymorphic;
			bool ok = is_type_polymorphic(t->Named.base, or_specialized);
			t->flags = flags;
			return ok;
		}

	case Type_Pointer:
		return is_type_polymorphic(t->Pointer.elem, or_specialized);

	case Type_MultiPointer:
		return is_type_polymorphic(t->MultiPointer.elem, or_specialized);

	case Type_SoaPointer:
		return is_type_polymorphic(t->SoaPointer.elem, or_specialized);

	case Type_EnumeratedArray:
		if (is_type_polymorphic(t->EnumeratedArray.index, or_specialized)) {
			return true;
		}
		return is_type_polymorphic(t->EnumeratedArray.elem, or_specialized);
	case Type_Array:
		if (t->Array.generic_count != nullptr) {
			return true;
		}
		return is_type_polymorphic(t->Array.elem, or_specialized);
	case Type_SimdVector:
		if (t->SimdVector.generic_count != nullptr) {
			return true;
		}
		return is_type_polymorphic(t->SimdVector.elem, or_specialized);
	case Type_DynamicArray:
		return is_type_polymorphic(t->DynamicArray.elem, or_specialized);
	case Type_Slice:
		return is_type_polymorphic(t->Slice.elem, or_specialized);

	case Type_Matrix:
		if (t->Matrix.generic_row_count != nullptr) {
			return true;
		}
		if (t->Matrix.generic_column_count != nullptr) {
			return true;
		}
		return is_type_polymorphic(t->Matrix.elem, or_specialized);

	case Type_Tuple:
		for (Entity *e : t->Tuple.variables) {
			if (e->kind == Entity_Constant) {
				if (e->Constant.value.kind != ExactValue_Invalid) {
					return or_specialized;
				}
			} else if (is_type_polymorphic(e->type, or_specialized)) {
				return true;
			}
		}
		break;

	case Type_Proc:
		if (t->Proc.is_polymorphic) {
			return true;
		}
		if (t->Proc.param_count > 0 &&
		    is_type_polymorphic(t->Proc.params, or_specialized)) {
			return true;
		}
		if (t->Proc.result_count > 0 &&
		    is_type_polymorphic(t->Proc.results, or_specialized)) {
			return true;
		}
		break;

	case Type_Enum:
		if (t->kind == Type_Enum) {
			if (t->Enum.base_type != nullptr) {
				return is_type_polymorphic(t->Enum.base_type, or_specialized);
			}
			return false;
		}
		break;
	case Type_Union:
		if (t->Union.is_polymorphic) {
			return true;
		}
		if (or_specialized && t->Union.is_poly_specialized) {
			return true;
		}
		// for_array(i, t->Union.variants) {
		//     if (is_type_polymorphic(t->Union.variants[i], or_specialized)) {
		//     	return true;
		//     }
		// }
		break;
	case Type_Struct:
		if (t->Struct.is_polymorphic) {
			return true;
		}
		if (or_specialized && t->Struct.is_poly_specialized) {
			return true;
		}
		break;

	case Type_Map:
		if (t->Map.key == nullptr || t->Map.value == nullptr) {
			return false;
		}
		if (is_type_polymorphic(t->Map.key, or_specialized)) {
			return true;
		}
		if (is_type_polymorphic(t->Map.value, or_specialized)) {
			return true;
		}
		break;

	case Type_BitSet:
		if (is_type_polymorphic(t->BitSet.elem, or_specialized)) {
			return true;
		}
		if (t->BitSet.underlying != nullptr &&
		    is_type_polymorphic(t->BitSet.underlying, or_specialized)) {
			return true;
		}
		break;
	}
	return false;
}


gb_internal bool type_has_nil(Type *t) {
	t = base_type(t);
	switch (t->kind) {
	case Type_Basic: {
		switch (t->Basic.kind) {
		case Basic_rawptr:
		case Basic_any:
			return true;
		case Basic_cstring:
			return true;
		case Basic_typeid:
			return true;
		}
		return false;
	} break;
	case Type_Enum:
	case Type_BitSet:
		return true;
	case Type_Slice:
	case Type_Proc:
	case Type_Pointer:
	case Type_SoaPointer:
	case Type_MultiPointer:
	case Type_DynamicArray:
	case Type_Map:
		return true;
	case Type_Union:
		return t->Union.kind != UnionType_no_nil;
	case Type_Struct:
		if (is_type_soa_struct(t)) {
			switch (t->Struct.soa_kind) {
			case StructSoa_Fixed:   return false;
			case StructSoa_Slice:   return true;
			case StructSoa_Dynamic: return true;
			}
		}
		return false;
	}
	return false;
}


gb_internal bool elem_type_can_be_constant(Type *t) {
	t = base_type(t);
	if (t == t_invalid) {
		return false;
	}
	if (is_type_any(t) || is_type_union(t) || is_type_raw_union(t)) {
		return false;
	}
	return true;
}

gb_internal bool is_type_lock_free(Type *t) {
	t = core_type(t);
	if (t == t_invalid) {
		return false;
	}
	i64 sz = type_size_of(t);
	// TODO(bill): Figure this out correctly
	return sz <= build_context.max_align;
}



gb_internal bool is_type_comparable(Type *t) {
	t = base_type(t);
	switch (t->kind) {
	case Type_Basic:
		switch (t->Basic.kind) {
		case Basic_UntypedNil:
		case Basic_any:
			return false;
		case Basic_rune:
			return true;
		case Basic_string:
			return true;
		case Basic_cstring:
			return true;
		case Basic_typeid:
			return true;
		}
		return true;
	case Type_Pointer:
		return true;
	case Type_SoaPointer:
		return true;
	case Type_MultiPointer:
		return true;
	case Type_Enum:
		return is_type_comparable(core_type(t));
	case Type_EnumeratedArray:
		return is_type_comparable(t->EnumeratedArray.elem);
	case Type_Array:
		return is_type_comparable(t->Array.elem);
	case Type_Proc:
		return true;
	case Type_Matrix:
		return is_type_comparable(t->Matrix.elem);

	case Type_BitSet:
		return true;

	case Type_Struct:
		if (t->Struct.soa_kind != StructSoa_None) {
			return false;
		}
		if (t->Struct.is_raw_union) {
			return is_type_simple_compare(t);
		}
		for_array(i, t->Struct.fields) {
			Entity *f = t->Struct.fields[i];
			if (!is_type_comparable(f->type)) {
				return false;
			}
		}
		return true;

	case Type_Union:
		for_array(i, t->Union.variants) {
			Type *v = t->Union.variants[i];
			if (!is_type_comparable(v)) {
				return false;
			}
		}
		return true;

	case Type_SimdVector:
		return true;

	case Type_BitField:
		return is_type_comparable(t->BitField.backing_type);
	}
	return false;
}

// NOTE(bill): type can be easily compared using memcmp
gb_internal bool is_type_simple_compare(Type *t) {
	t = core_type(t);
	switch (t->kind) {
	case Type_Array:
		return is_type_simple_compare(t->Array.elem);

	case Type_EnumeratedArray:
		return is_type_simple_compare(t->EnumeratedArray.elem);

	case Type_Basic:
		if (t->Basic.flags & BasicFlag_SimpleCompare) {
			return true;
		}
		if (t->Basic.kind == Basic_typeid) {
			return true;
		}
		return false;

	case Type_Pointer:
	case Type_MultiPointer:
	case Type_SoaPointer:
	case Type_Proc:
	case Type_BitSet:
		return true;

	case Type_Matrix:
		return is_type_simple_compare(t->Matrix.elem);

	case Type_Struct:
		for_array(i, t->Struct.fields) {
			Entity *f = t->Struct.fields[i];
			if (!is_type_simple_compare(f->type)) {
				return false;
			}
		}
		return true;

	case Type_Union:
		for_array(i, t->Union.variants) {
			Type *v = t->Union.variants[i];
			if (!is_type_simple_compare(v)) {
				return false;
			}
		}
		// make it dumb on purpose
		return t->Union.variants.count == 1;

	case Type_SimdVector:
		return is_type_simple_compare(t->SimdVector.elem);

	}

	return false;
}

gb_internal bool is_type_load_safe(Type *type) {
	GB_ASSERT(type != nullptr);
	type = core_type(core_array_type(type));
	switch (type->kind) {
	case Type_Basic:
		return (type->Basic.flags & (BasicFlag_Boolean|BasicFlag_Numeric|BasicFlag_Rune)) != 0;

	case Type_BitSet:
		if (type->BitSet.underlying) {
			return is_type_load_safe(type->BitSet.underlying);
		}
		return true;

	case Type_Pointer:
	case Type_MultiPointer:
	case Type_Slice:
	case Type_DynamicArray:
	case Type_Proc:
	case Type_SoaPointer:
		return false;

	case Type_Enum:
	case Type_EnumeratedArray:
	case Type_Array:
	case Type_SimdVector:
	case Type_Matrix:
		GB_PANIC("should never be hit");
		return false;

	case Type_Struct:
		for_array(i, type->Struct.fields) {
			if (!is_type_load_safe(type->Struct.fields[i]->type)) {
				return false;
			}
		}
		return type_size_of(type) > 0;
	case Type_Union:
		for_array(i, type->Union.variants) {
			if (!is_type_load_safe(type->Union.variants[i])) {
				return false;
			}
		}
		return type_size_of(type) > 0;
	}
	return false;
}

gb_internal String lookup_subtype_polymorphic_field(Type *dst, Type *src) {
	Type *prev_src = src;
	// Type *prev_dst = dst;
	src = base_type(type_deref(src));
	// dst = base_type(type_deref(dst));
	bool src_is_ptr = src != prev_src;
	// bool dst_is_ptr = dst != prev_dst;

	GB_ASSERT(is_type_struct(src) || is_type_union(src));
	for_array(i, src->Struct.fields) {
		Entity *f = src->Struct.fields[i];
		if (f->kind == Entity_Variable && f->flags & EntityFlags_IsSubtype) {
			if (are_types_identical(dst, f->type)) {
				return f->token.string;
			}
			if (src_is_ptr && is_type_pointer(dst)) {
				if (are_types_identical(type_deref(dst), f->type)) {
					return f->token.string;
				}
			}
			if ((f->flags & EntityFlag_Using) != 0 && is_type_struct(f->type)) {
				String name = lookup_subtype_polymorphic_field(dst, f->type);
				if (name.len > 0) {
					return name;
				}
			}
		}
	}
	return str_lit("");
}

gb_internal bool lookup_subtype_polymorphic_selection(Type *dst, Type *src, Selection *sel) {
	Type *prev_src = src;
	// Type *prev_dst = dst;
	src = base_type(type_deref(src));
	// dst = base_type(type_deref(dst));
	bool src_is_ptr = src != prev_src;
	// bool dst_is_ptr = dst != prev_dst;

	GB_ASSERT(is_type_struct(src) || is_type_union(src));
	for_array(i, src->Struct.fields) {
		Entity *f = src->Struct.fields[i];
		if (f->kind == Entity_Variable && f->flags & EntityFlags_IsSubtype) {
			if (are_types_identical(dst, f->type)) {
				array_add(&sel->index, cast(i32)i);
				sel->entity = f;
				return true;
			}
			if (src_is_ptr && is_type_pointer(dst)) {
				if (are_types_identical(type_deref(dst), f->type)) {
					array_add(&sel->index, cast(i32)i);
					sel->indirect = true;
					sel->entity = f;
					return true;
				}
			}
			if ((f->flags & EntityFlag_Using) != 0 && is_type_struct(f->type)) {
				String name = lookup_subtype_polymorphic_field(dst, f->type);
				if (name.len > 0) {
					array_add(&sel->index, cast(i32)i);
					return lookup_subtype_polymorphic_selection(dst, f->type, sel);
				}
			}
		}
	}
	return false;
}



gb_internal bool are_types_identical_internal(Type *x, Type *y, bool check_tuple_names);

gb_internal bool are_types_identical(Type *x, Type *y) {
	if (x == y) {
		return true;
	}

	if ((x == nullptr && y != nullptr) ||
	    (x != nullptr && y == nullptr)) {
		return false;
	}

	if (x->kind == Type_Named) {
		Entity *e = x->Named.type_name;
		if (e->TypeName.is_type_alias) {
			x = x->Named.base;
		}
	}
	if (y->kind == Type_Named) {
		Entity *e = y->Named.type_name;
		if (e->TypeName.is_type_alias) {
			y = y->Named.base;
		}
	}
	if (x == nullptr || y == nullptr || x->kind != y->kind) {
		return false;
	}

	return are_types_identical_internal(x, y, false);
}
gb_internal bool are_types_identical_unique_tuples(Type *x, Type *y) {
	if (x == y) {
		return true;
	}

	if (!x | !y) {
		return false;
	}

	if (x->kind == Type_Named) {
		Entity *e = x->Named.type_name;
		if (e->TypeName.is_type_alias) {
			x = x->Named.base;
		}
	}
	if (y->kind == Type_Named) {
		Entity *e = y->Named.type_name;
		if (e->TypeName.is_type_alias) {
			y = y->Named.base;
		}
	}
	if (x->kind != y->kind) {
		return false;
	}

	return are_types_identical_internal(x, y, true);
}


gb_internal bool are_types_identical_internal(Type *x, Type *y, bool check_tuple_names) {
	if (x == y) {
		return true;
	}

	if (!x | !y) {
		return false;
	}

	#if 0
	if (x->kind == Type_Named) {
		Entity *e = x->Named.type_name;
		if (e->TypeName.is_type_alias) {
			x = x->Named.base;
		}
	}
	if (y->kind == Type_Named) {
		Entity *e = y->Named.type_name;
		if (e->TypeName.is_type_alias) {
			y = y->Named.base;
		}
	}
	if (x->kind != y->kind) {
		return false;
	}
	#endif

	switch (x->kind) {
	case Type_Generic:
		return are_types_identical(x->Generic.specialized, y->Generic.specialized);

	case Type_Basic:
		return x->Basic.kind == y->Basic.kind;

	case Type_EnumeratedArray:
		return are_types_identical(x->EnumeratedArray.index, y->EnumeratedArray.index) &&
		       are_types_identical(x->EnumeratedArray.elem,  y->EnumeratedArray.elem);

	case Type_Array:
		return (x->Array.count == y->Array.count) && are_types_identical(x->Array.elem, y->Array.elem);

	case Type_Matrix:
		return x->Matrix.row_count == y->Matrix.row_count &&
		       x->Matrix.column_count == y->Matrix.column_count &&
		       x->Matrix.is_row_major == y->Matrix.is_row_major &&
		       are_types_identical(x->Matrix.elem, y->Matrix.elem);

	case Type_DynamicArray:
		return are_types_identical(x->DynamicArray.elem, y->DynamicArray.elem);

	case Type_Slice:
		return are_types_identical(x->Slice.elem, y->Slice.elem);

	case Type_BitSet:
		if (are_types_identical(x->BitSet.elem, y->BitSet.elem) &&
		    are_types_identical(x->BitSet.underlying, y->BitSet.underlying)) {
		    	if (is_type_enum(x->BitSet.elem)) {
		    		return true;
		    	}
		    	return x->BitSet.lower == y->BitSet.lower && x->BitSet.upper == y->BitSet.upper;
		}
		return false;


	case Type_Enum:
		if (x == y) {
			return true;
		}
		if (x->Enum.fields.count != y->Enum.fields.count) {
			return false;
		}
		if (!are_types_identical(x->Enum.base_type, y->Enum.base_type)) {
			return false;
		}
		if (x->Enum.min_value_index != y->Enum.min_value_index) {
			return false;
		}
		if (x->Enum.max_value_index != y->Enum.max_value_index) {
			return false;
		}

		for (isize i = 0; i < x->Enum.fields.count; i++) {
			Entity *a = x->Enum.fields[i];
			Entity *b = y->Enum.fields[i];
			if (a->token.string != b->token.string) {
				return false;
			}
			GB_ASSERT(a->kind == b->kind);
			GB_ASSERT(a->kind == Entity_Constant);
			bool same = compare_exact_values(Token_CmpEq, a->Constant.value, b->Constant.value);
			if (!same) {
				return false;
			}
		}

		return true;

	case Type_Union:
		if (x->Union.variants.count == y->Union.variants.count &&
		    x->Union.kind == y->Union.kind) {

			if (x->Union.custom_align != y->Union.custom_align) {
				if (type_align_of(x) != type_align_of(y)) {
					return false;
				}
			}

			// NOTE(bill): zeroth variant is nullptr
			for_array(i, x->Union.variants) {
				if (!are_types_identical(x->Union.variants[i], y->Union.variants[i])) {
					return false;
				}
			}
			return true;
		}
		break;

	case Type_Struct:
		if (x->Struct.is_raw_union == y->Struct.is_raw_union &&
		    x->Struct.fields.count == y->Struct.fields.count &&
		    x->Struct.is_packed    == y->Struct.is_packed &&
		    x->Struct.soa_kind == y->Struct.soa_kind &&
		    x->Struct.soa_count == y->Struct.soa_count &&
		    are_types_identical(x->Struct.soa_elem, y->Struct.soa_elem)) {

			if (x->Struct.custom_align != y->Struct.custom_align) {
				if (type_align_of(x) != type_align_of(y)) {
					return false;
				}
			}

			for_array(i, x->Struct.fields) {
				Entity *xf = x->Struct.fields[i];
				Entity *yf = y->Struct.fields[i];
				if (xf->kind != yf->kind) {
					return false;
				}
				if (!are_types_identical(xf->type, yf->type)) {
					return false;
				}
				if (xf->token.string != yf->token.string) {
					return false;
				}
				if (x->Struct.tags[i] != y->Struct.tags[i]) {
					return false;
				}
				u64 xf_flags = (xf->flags&EntityFlags_IsSubtype);
				u64 yf_flags = (yf->flags&EntityFlags_IsSubtype);
				if (xf_flags != yf_flags) {
					return false;
				}
			}
			// TODO(bill): Which is the correct logic here?
			// return are_types_identical(x->Struct.polymorphic_params, y->Struct.polymorphic_params);
			return true;
		}
		break;

	case Type_Pointer:
		return are_types_identical(x->Pointer.elem, y->Pointer.elem);

	case Type_MultiPointer:
		return are_types_identical(x->MultiPointer.elem, y->MultiPointer.elem);

	case Type_SoaPointer:
		return are_types_identical(x->SoaPointer.elem, y->SoaPointer.elem);

	case Type_Named:
		return x->Named.type_name == y->Named.type_name;

	case Type_Tuple:
		if (x->Tuple.variables.count == y->Tuple.variables.count &&
		    x->Tuple.is_packed == y->Tuple.is_packed) {
			for_array(i, x->Tuple.variables) {
				Entity *xe = x->Tuple.variables[i];
				Entity *ye = y->Tuple.variables[i];
				if (xe->kind != ye->kind || !are_types_identical(xe->type, ye->type)) {
					return false;
				}
				if (check_tuple_names) {
					if (xe->token.string != ye->token.string) {
						return false;
					}
				}
				if (xe->kind == Entity_Constant && !compare_exact_values(Token_CmpEq, xe->Constant.value, ye->Constant.value)) {
					// NOTE(bill): This is needed for polymorphic procedures
					return false;
				}
			}
			return true;
		}
		break;

	case Type_Proc:
		return x->Proc.calling_convention == y->Proc.calling_convention &&
		       x->Proc.c_vararg    == y->Proc.c_vararg    &&
		       x->Proc.variadic    == y->Proc.variadic    &&
		       x->Proc.diverging   == y->Proc.diverging   &&
		       x->Proc.optional_ok == y->Proc.optional_ok &&
		       are_types_identical_internal(x->Proc.params, y->Proc.params, check_tuple_names) &&
		       are_types_identical_internal(x->Proc.results, y->Proc.results, check_tuple_names);

	case Type_Map:
		return are_types_identical(x->Map.key,   y->Map.key) &&
		       are_types_identical(x->Map.value, y->Map.value);

	case Type_SimdVector:
		if (x->SimdVector.count == y->SimdVector.count) {
			return are_types_identical(x->SimdVector.elem, y->SimdVector.elem);
		}
		break;

	case Type_BitField:
		if (are_types_identical(x->BitField.backing_type, y->BitField.backing_type) &&
		    x->BitField.fields.count == y->BitField.fields.count) {
			for_array(i, x->BitField.fields) {
				Entity *a = x->BitField.fields[i];
				Entity *b = y->BitField.fields[i];
				if (!are_types_identical(a->type, b->type)) {
					return false;
				}
				if (a->token.string != b->token.string) {
					return false;
				}
				if (x->BitField.bit_sizes[i] != y->BitField.bit_sizes[i]) {
					return false;
				}
				if (x->BitField.bit_offsets[i] != y->BitField.bit_offsets[i]) {
					return false;
				}
			}
			return true;
		}
		break;
	}

	return false;
}

gb_internal Type *default_type(Type *type) {
	if (type == nullptr) {
		return t_invalid;
	}
	if (type->kind == Type_Basic) {
		switch (type->Basic.kind) {
		case Basic_UntypedBool:       return t_bool;
		case Basic_UntypedInteger:    return t_int;
		case Basic_UntypedFloat:      return t_f64;
		case Basic_UntypedComplex:    return t_complex128;
		case Basic_UntypedQuaternion: return t_quaternion256;
		case Basic_UntypedString:     return t_string;
		case Basic_UntypedRune:       return t_rune;
		}
	} else if (type->kind == Type_Generic) {
		if (type->Generic.specialized) {
			return default_type(type->Generic.specialized);
		}
	}
	return type;
}

// See https://en.cppreference.com/w/c/language/conversion#Default_argument_promotions
gb_internal Type *c_vararg_promote_type(Type *type) {
	GB_ASSERT(type != nullptr);

	Type *core = core_type(type);
	GB_ASSERT(core->kind != Type_BitSet);

	if (core->kind == Type_Basic) {
		switch (core->Basic.kind) {
		case Basic_f16:
		case Basic_f32:
		case Basic_UntypedFloat:
			return t_f64;
		case Basic_f16le:
		case Basic_f32le:
			return t_f64le;
		case Basic_f16be:
		case Basic_f32be:
			return t_f64be;

		case Basic_UntypedBool:
		case Basic_bool:
		case Basic_b8:
		case Basic_b16:
		case Basic_i8:
		case Basic_i16:
		case Basic_u8:
		case Basic_u16:
			return t_i32;

		case Basic_i16le:
		case Basic_u16le:
			return t_i32le;

		case Basic_i16be:
		case Basic_u16be:
			return t_i32be;
		}
	}

	return type;
}

gb_internal bool union_variant_index_types_equal(Type *v, Type *vt) {
	if (are_types_identical(v, vt)) {
		return true;
	}
	if (is_type_proc(v) && is_type_proc(vt)) {
		return are_types_identical(base_type(v), base_type(vt));
	}
	return false;
}

gb_internal i64 union_variant_index(Type *u, Type *v) {
	u = base_type(u);
	GB_ASSERT(u->kind == Type_Union);

	for_array(i, u->Union.variants) {
		Type *vt = u->Union.variants[i];
		if (union_variant_index_types_equal(v, vt)) {
			if (u->Union.kind == UnionType_no_nil) {
				return cast(i64)(i+0);
			} else {
				return cast(i64)(i+1);
			}
		}
	}
	return 0;
}

gb_internal i64 union_tag_size(Type *u) {
	u = base_type(u);
	GB_ASSERT(u->kind == Type_Union);
	if (u->Union.tag_size > 0) {
		return u->Union.tag_size;
	}

	u64 n = cast(u64)u->Union.variants.count;
	if (n == 0) {
		return 0;
	}

	i64 max_align = 1;

	if (u->Union.variants.count < 1ull<<8) {
		max_align = 1;
	} else if (u->Union.variants.count < 1ull<<16) {
		max_align = 2;
	} else if (u->Union.variants.count < 1ull<<32) {
		max_align = 4;
	} else {
		compiler_error("how many variants do you have?! %lld", cast(long long)u->Union.variants.count);
	}

	if (u->Union.custom_align > 0) {
		max_align = gb_max(max_align, u->Union.custom_align);
	} else {
		for_array(i, u->Union.variants) {
			Type *variant_type = u->Union.variants[i];
			i64 align = type_align_of(variant_type);
			if (max_align < align) {
				max_align = align;
			}
		}
	}

	u->Union.tag_size = cast(i16)gb_min3(max_align, build_context.max_align, 8);
	return u->Union.tag_size;
}

gb_internal Type *union_tag_type(Type *u) {
	i64 s = union_tag_size(u);
	switch (s) {
	case  0: return  t_u8;
	case  1: return  t_u8;
	case  2: return  t_u16;
	case  4: return  t_u32;
	case  8: return  t_u64;
	}
	GB_PANIC("Invalid union_tag_size");
	return t_uint;
}

gb_internal int matched_target_features(TypeProc *t) {
	if (t->require_target_feature.len == 0) {
		return 0;
	}

	int matches = 0;
	String_Iterator it = {t->require_target_feature, 0};
	for (;;) {
		String str = string_split_iterator(&it, ',');
		if (str == "") break;
		if (check_target_feature_is_valid_for_target_arch(str, nullptr)) {
			matches += 1;
		}
	}
	return matches;
}

enum ProcTypeOverloadKind {
	ProcOverload_Identical, // The types are identical

	ProcOverload_CallingConvention,
	ProcOverload_ParamCount,
	ProcOverload_ParamVariadic,
	ProcOverload_ParamTypes,
	ProcOverload_ResultCount,
	ProcOverload_ResultTypes,
	ProcOverload_Polymorphic,
	ProcOverload_TargetFeatures,

	ProcOverload_NotProcedure,

};

gb_internal ProcTypeOverloadKind are_proc_types_overload_safe(Type *x, Type *y) {
	if (x == nullptr && y == nullptr) return ProcOverload_NotProcedure;
	if (x == nullptr && y != nullptr) return ProcOverload_NotProcedure;
	if (x != nullptr && y == nullptr) return ProcOverload_NotProcedure;
 	if (!is_type_proc(x))       return ProcOverload_NotProcedure;
 	if (!is_type_proc(y))       return ProcOverload_NotProcedure;

	TypeProc px = base_type(x)->Proc;
	TypeProc py = base_type(y)->Proc;


	// if (px.calling_convention != py.calling_convention) {
		// return ProcOverload_CallingConvention;
	// }

	// if (px.is_polymorphic != py.is_polymorphic) {
		// return ProcOverload_Polymorphic;
	// }

	if (px.param_count != py.param_count) {
		return ProcOverload_ParamCount;
	}

	for (isize i = 0; i < px.param_count; i++) {
		Entity *ex = px.params->Tuple.variables[i];
		Entity *ey = py.params->Tuple.variables[i];
		if (!are_types_identical(ex->type, ey->type)) {
			return ProcOverload_ParamTypes;
		}
	}
	// IMPORTANT TODO(bill): Determine the rules for overloading procedures with variadic parameters
	if (px.variadic != py.variadic) {
		return ProcOverload_ParamVariadic;
	}


	if (px.is_polymorphic != py.is_polymorphic) {
		return ProcOverload_Polymorphic;
	}

	if (px.result_count != py.result_count) {
		return ProcOverload_ResultCount;
	}

	for (isize i = 0; i < px.result_count; i++) {
		Entity *ex = px.results->Tuple.variables[i];
		Entity *ey = py.results->Tuple.variables[i];
		if (!are_types_identical(ex->type, ey->type)) {
			return ProcOverload_ResultTypes;
		}
	}

	if (matched_target_features(&px) != matched_target_features(&py)) {
		return ProcOverload_TargetFeatures;
	}

	if (px.params != nullptr && py.params != nullptr) {
		Entity *ex = px.params->Tuple.variables[0];
		Entity *ey = py.params->Tuple.variables[0];
		bool ok = are_types_identical(ex->type, ey->type);
		if (ok) {
		}
	}

	return ProcOverload_Identical;
}





gb_internal Selection lookup_field_with_selection(Type *type_, String field_name, bool is_type, Selection sel, bool allow_blank_ident=false);

gb_internal Selection lookup_field(Type *type_, String field_name, bool is_type, bool allow_blank_ident=false) {
	return lookup_field_with_selection(type_, field_name, is_type, empty_selection, allow_blank_ident);
}

gb_internal Selection lookup_field_from_index(Type *type, i64 index) {
	GB_ASSERT(is_type_struct(type) || is_type_union(type) || is_type_tuple(type));
	type = base_type(type);

	gbAllocator a = permanent_allocator();
	isize max_count = 0;
	switch (type->kind) {
	case Type_Struct:
		wait_signal_until_available(&type->Struct.fields_wait_signal);
		max_count = type->Struct.fields.count;
		break;
	case Type_Tuple:    max_count = type->Tuple.variables.count; break;
	}

	if (index >= max_count) {
		return empty_selection;
	}

	switch (type->kind) {
	case Type_Struct: {
		wait_signal_until_available(&type->Struct.fields_wait_signal);
		for (isize i = 0; i < max_count; i++) {
			Entity *f = type->Struct.fields[i];
			if (f->kind == Entity_Variable) {
				if (f->Variable.field_index == index) {
					auto sel_array = array_make<i32>(a, 1);
					sel_array[0] = cast(i32)i;
					return make_selection(f, sel_array, false);
				}
			}
		}
	} break;

	case Type_Tuple:
		for (isize i = 0; i < max_count; i++) {
			Entity *f = type->Tuple.variables[i];
			if (i == index) {
				auto sel_array = array_make<i32>(a, 1);
				sel_array[0] = cast(i32)i;
				return make_selection(f, sel_array, false);
			}
		}
		break;
	}

	GB_PANIC("Illegal index");
	return empty_selection;
}

gb_internal Entity *scope_lookup_current(Scope *s, String const &name);
gb_internal bool has_type_got_objc_class_attribute(Type *t);

gb_internal Selection lookup_field_with_selection(Type *type_, String field_name, bool is_type, Selection sel, bool allow_blank_ident) {
	GB_ASSERT(type_ != nullptr);

	if (!allow_blank_ident && is_blank_ident(field_name)) {
		return empty_selection;
	}

	Type *type = type_deref(type_);
	bool is_ptr = type != type_;
	sel.indirect = sel.indirect || is_ptr;

	Type *original_type = type;

	type = base_type(type);

	if (is_type) {
		if (has_type_got_objc_class_attribute(original_type) && original_type->kind == Type_Named) {
			Entity *e = original_type->Named.type_name;
			GB_ASSERT(e->kind == Entity_TypeName);
			if (e->TypeName.objc_metadata) {
				auto *md = e->TypeName.objc_metadata;
				mutex_lock(md->mutex);
				defer (mutex_unlock(md->mutex));
				for (TypeNameObjCMetadataEntry const &entry : md->type_entries) {
					GB_ASSERT(entry.entity->kind == Entity_Procedure || entry.entity->kind == Entity_ProcGroup);
					if (entry.name == field_name) {
						sel.entity = entry.entity;
						sel.pseudo_field = true;
						return sel;
					}
				}
			}
			if (type->kind == Type_Struct) {
				// wait_signal_until_available(&type->Struct.fields_wait_signal);
				isize field_count = type->Struct.fields.count;
				if (field_count != 0) for_array(i, type->Struct.fields) {
					Entity *f = type->Struct.fields[i];
					if (f->flags&EntityFlag_Using) {
						sel = lookup_field_with_selection(f->type, field_name, is_type, sel, allow_blank_ident);
						if (sel.entity) {
							return sel;
						}
					}
				}
			}
		}

		if (is_type_enum(type)) {
			// NOTE(bill): These may not have been added yet, so check in case
			for_array(i, type->Enum.fields) {
				Entity *f = type->Enum.fields[i];
				GB_ASSERT(f->kind == Entity_Constant);
				String str = f->token.string;

				if (field_name == str) {
					sel.entity = f;
					// selection_add_index(&sel, i);
					return sel;
				}
			}
		}

		if (type->kind == Type_Struct) {
			// wait_signal_until_available(&type->Struct.fields_wait_signal);
			Scope *s = type->Struct.scope;
			if (s != nullptr) {
				Entity *found = scope_lookup_current(s, field_name);
				if (found != nullptr && found->kind != Entity_Variable) {
					sel.entity = found;
					return sel;
				}
			}
		} else if (type->kind == Type_Union) {
			Scope *s = type->Union.scope;
			if (s != nullptr) {
				Entity *found = scope_lookup_current(s, field_name);
				if (found != nullptr && found->kind != Entity_Variable) {
					sel.entity = found;
					return sel;
				}
			}
		} else if (type->kind == Type_BitSet) {
			return lookup_field_with_selection(type->BitSet.elem, field_name, true, sel, allow_blank_ident);
		}


		if (type->kind == Type_Generic && type->Generic.specialized != nullptr) {
			Type *specialized = type->Generic.specialized;
			return lookup_field_with_selection(specialized, field_name, is_type, sel, allow_blank_ident);
		}

	} else if (type->kind == Type_Union) {

	} else if (type->kind == Type_Struct) {
		if (has_type_got_objc_class_attribute(original_type) && original_type->kind == Type_Named) {
			Entity *e = original_type->Named.type_name;
			GB_ASSERT(e->kind == Entity_TypeName);
			if (e->TypeName.objc_metadata) {
				auto *md = e->TypeName.objc_metadata;
				mutex_lock(md->mutex);
				defer (mutex_unlock(md->mutex));
				for (TypeNameObjCMetadataEntry const &entry : md->value_entries) {
					GB_ASSERT(entry.entity->kind == Entity_Procedure || entry.entity->kind == Entity_ProcGroup);
					if (entry.name == field_name) {
						sel.entity = entry.entity;
						sel.pseudo_field = true;
						return sel;
					}
				}
			}

			Type *objc_ivar_type = e->TypeName.objc_ivar;
			if (objc_ivar_type != nullptr) {
				sel = lookup_field_with_selection(objc_ivar_type, field_name, false, sel, allow_blank_ident);
				if (sel.entity != nullptr) {
					sel.pseudo_field = true;
					return sel;
				}
			}
		}

		if (is_type_polymorphic(type)) {
			// NOTE(bill): A polymorphic struct has no fields, this only hits in the case of an error
			return sel;
		}
		wait_signal_until_available(&type->Struct.fields_wait_signal);
		isize field_count = type->Struct.fields.count;
		if (field_count != 0) for_array(i, type->Struct.fields) {
			Entity *f = type->Struct.fields[i];
			if (f->kind != Entity_Variable || (f->flags & EntityFlag_Field) == 0) {
				continue;
			}
			String str = f->token.string;
			if (field_name == str) {
				selection_add_index(&sel, i);  // HACK(bill): Leaky memory
				sel.entity = f;
				return sel;
			}

			if (f->flags & EntityFlag_Using) {
				isize prev_count = sel.index.count;
				bool prev_indirect = sel.indirect;
				selection_add_index(&sel, i); // HACK(bill): Leaky memory

				sel = lookup_field_with_selection(f->type, field_name, is_type, sel, allow_blank_ident);

				if (sel.entity != nullptr) {
					if (is_type_pointer(f->type)) {
						sel.indirect = true;
					}
					return sel;
				}
				sel.index.count = prev_count;
				sel.indirect = prev_indirect;
			}
		}

		bool is_soa = type->Struct.soa_kind != StructSoa_None;
		bool is_soa_of_array = is_soa && is_type_array(type->Struct.soa_elem);

		if (is_soa_of_array) {
			String mapped_field_name = {};
			     if (field_name == "r") mapped_field_name = str_lit("x");
			else if (field_name == "g") mapped_field_name = str_lit("y");
			else if (field_name == "b") mapped_field_name = str_lit("z");
			else if (field_name == "a") mapped_field_name = str_lit("w");
			return lookup_field_with_selection(type, mapped_field_name, is_type, sel, allow_blank_ident);
		}
	} else if (type->kind == Type_BitField) {
		for_array(i, type->BitField.fields) {
			Entity *f = type->BitField.fields[i];
			if (f->kind != Entity_Variable || (f->flags & EntityFlag_Field) == 0) {
				continue;
			}
			String str = f->token.string;
			if (field_name == str) {
				selection_add_index(&sel, i);  // HACK(bill): Leaky memory
				sel.entity = f;
				sel.is_bit_field = true;
				return sel;
			}
		}

	} else if (type->kind == Type_Basic) {
		switch (type->Basic.kind) {
		case Basic_any: {
		#if 1
			String data_str = str_lit("data");
			String id_str = str_lit("id");
			gb_local_persist Entity *entity__any_data = alloc_entity_field(nullptr, make_token_ident(data_str), t_rawptr, false, 0);
			gb_local_persist Entity *entity__any_id = alloc_entity_field(nullptr, make_token_ident(id_str), t_typeid, false, 1);

			if (field_name == data_str) {
				selection_add_index(&sel, 0);
				sel.entity = entity__any_data;
				return sel;
			} else if (field_name == id_str) {
				selection_add_index(&sel, 1);
				sel.entity = entity__any_id;
				return sel;
			}
		#endif
		} break;

		case Basic_quaternion64: {
			// @QuaternionLayout
			gb_local_persist String w = str_lit("w");
			gb_local_persist String x = str_lit("x");
			gb_local_persist String y = str_lit("y");
			gb_local_persist String z = str_lit("z");
			gb_local_persist Entity *entity__w = alloc_entity_field(nullptr, make_token_ident(w), t_f16, false, 3);
			gb_local_persist Entity *entity__x = alloc_entity_field(nullptr, make_token_ident(x), t_f16, false, 0);
			gb_local_persist Entity *entity__y = alloc_entity_field(nullptr, make_token_ident(y), t_f16, false, 1);
			gb_local_persist Entity *entity__z = alloc_entity_field(nullptr, make_token_ident(z), t_f16, false, 2);
			if (field_name == w) {
				selection_add_index(&sel, 3);
				sel.entity = entity__w;
				return sel;
			} else if (field_name == x) {
				selection_add_index(&sel, 0);
				sel.entity = entity__x;
				return sel;
			} else if (field_name == y) {
				selection_add_index(&sel, 1);
				sel.entity = entity__y;
				return sel;
			} else if (field_name == z) {
				selection_add_index(&sel, 2);
				sel.entity = entity__z;
				return sel;
			}
		} break;

		case Basic_quaternion128: {
			// @QuaternionLayout
			gb_local_persist String w = str_lit("w");
			gb_local_persist String x = str_lit("x");
			gb_local_persist String y = str_lit("y");
			gb_local_persist String z = str_lit("z");
			gb_local_persist Entity *entity__w = alloc_entity_field(nullptr, make_token_ident(w), t_f32, false, 3);
			gb_local_persist Entity *entity__x = alloc_entity_field(nullptr, make_token_ident(x), t_f32, false, 0);
			gb_local_persist Entity *entity__y = alloc_entity_field(nullptr, make_token_ident(y), t_f32, false, 1);
			gb_local_persist Entity *entity__z = alloc_entity_field(nullptr, make_token_ident(z), t_f32, false, 2);
			if (field_name == w) {
				selection_add_index(&sel, 3);
				sel.entity = entity__w;
				return sel;
			} else if (field_name == x) {
				selection_add_index(&sel, 0);
				sel.entity = entity__x;
				return sel;
			} else if (field_name == y) {
				selection_add_index(&sel, 1);
				sel.entity = entity__y;
				return sel;
			} else if (field_name == z) {
				selection_add_index(&sel, 2);
				sel.entity = entity__z;
				return sel;
			}
		} break;

		case Basic_quaternion256: {
			// @QuaternionLayout
			gb_local_persist String w = str_lit("w");
			gb_local_persist String x = str_lit("x");
			gb_local_persist String y = str_lit("y");
			gb_local_persist String z = str_lit("z");
			gb_local_persist Entity *entity__w = alloc_entity_field(nullptr, make_token_ident(w), t_f64, false, 3);
			gb_local_persist Entity *entity__x = alloc_entity_field(nullptr, make_token_ident(x), t_f64, false, 0);
			gb_local_persist Entity *entity__y = alloc_entity_field(nullptr, make_token_ident(y), t_f64, false, 1);
			gb_local_persist Entity *entity__z = alloc_entity_field(nullptr, make_token_ident(z), t_f64, false, 2);
			if (field_name == w) {
				selection_add_index(&sel, 3);
				sel.entity = entity__w;
				return sel;
			} else if (field_name == x) {
				selection_add_index(&sel, 0);
				sel.entity = entity__x;
				return sel;
			} else if (field_name == y) {
				selection_add_index(&sel, 1);
				sel.entity = entity__y;
				return sel;
			} else if (field_name == z) {
				selection_add_index(&sel, 2);
				sel.entity = entity__z;
				return sel;
			}
		} break;

		case Basic_UntypedQuaternion: {
			// @QuaternionLayout
			gb_local_persist String w = str_lit("w");
			gb_local_persist String x = str_lit("x");
			gb_local_persist String y = str_lit("y");
			gb_local_persist String z = str_lit("z");
			gb_local_persist Entity *entity__w = alloc_entity_field(nullptr, make_token_ident(w), t_untyped_float, false, 3);
			gb_local_persist Entity *entity__x = alloc_entity_field(nullptr, make_token_ident(x), t_untyped_float, false, 0);
			gb_local_persist Entity *entity__y = alloc_entity_field(nullptr, make_token_ident(y), t_untyped_float, false, 1);
			gb_local_persist Entity *entity__z = alloc_entity_field(nullptr, make_token_ident(z), t_untyped_float, false, 2);
			if (field_name == w) {
				selection_add_index(&sel, 3);
				sel.entity = entity__w;
				return sel;
			} else if (field_name == x) {
				selection_add_index(&sel, 0);
				sel.entity = entity__x;
				return sel;
			} else if (field_name == y) {
				selection_add_index(&sel, 1);
				sel.entity = entity__y;
				return sel;
			} else if (field_name == z) {
				selection_add_index(&sel, 2);
				sel.entity = entity__z;
				return sel;
			}
		} break;

		}

		return sel;
	} else if (type->kind == Type_DynamicArray) {
		GB_ASSERT(t_allocator != nullptr);
		String allocator_str = str_lit("allocator");
		gb_local_persist Entity *entity__allocator = alloc_entity_field(nullptr, make_token_ident(allocator_str), t_allocator, false, 3);

		if (field_name == allocator_str) {
			selection_add_index(&sel, 3);
			sel.entity = entity__allocator;
			return sel;
		}
	} else if (type->kind == Type_Map) {
		GB_ASSERT(t_allocator != nullptr);
		String allocator_str = str_lit("allocator");
		gb_local_persist Entity *entity__allocator = alloc_entity_field(nullptr, make_token_ident(allocator_str), t_allocator, false, 2);

		if (field_name == allocator_str) {
			selection_add_index(&sel, 2);
			sel.entity = entity__allocator;
			return sel;
		}


#define _ARRAY_FIELD_CASE_IF(_length, _name) \
	if (field_name == (_name)) { \
		selection_add_index(&sel, (_length)-1); \
		sel.entity = alloc_entity_array_elem(nullptr, make_token_ident(str_lit(_name)), elem, (_length)-1); \
		return sel; \
	}
#define _ARRAY_FIELD_CASE(_length, _name0, _name1) \
case (_length): \
	_ARRAY_FIELD_CASE_IF(_length, _name0); \
	_ARRAY_FIELD_CASE_IF(_length, _name1); \
	/*fallthrough*/


	} else if (type->kind == Type_Array) {

		Type *elem = type->Array.elem;

		if (type->Array.count <= 4) {
			// HACK(bill): Memory leak
			switch (type->Array.count) {

			_ARRAY_FIELD_CASE(4, "w", "a");
			_ARRAY_FIELD_CASE(3, "z", "b");
			_ARRAY_FIELD_CASE(2, "y", "g");
			_ARRAY_FIELD_CASE(1, "x", "r");
			default: break;
			}
		}
	} else if (type->kind == Type_SimdVector) {

		Type *elem = type->SimdVector.elem;
		if (type->SimdVector.count <= 4) {
			// HACK(bill): Memory leak
			switch (type->SimdVector.count) {
			_ARRAY_FIELD_CASE(4, "w", "a");
			_ARRAY_FIELD_CASE(3, "z", "b");
			_ARRAY_FIELD_CASE(2, "y", "g");
			_ARRAY_FIELD_CASE(1, "x", "r");
			default: break;
			}
		}
	}

#undef _ARRAY_FIELD_CASE
#undef _ARRAY_FIELD_CASE

	return sel;
}

gb_internal bool are_struct_fields_reordered(Type *type) {
	type = base_type(type);
	GB_ASSERT(type->kind == Type_Struct);
	type_set_offsets(type);
	if (type->Struct.fields.count == 0) {
		return false;
	}
	GB_ASSERT(type->Struct.offsets != nullptr);

	i64 prev_offset = 0;
	for_array(i, type->Struct.fields) {
		i64 offset = type->Struct.offsets[i];
		if (prev_offset > offset) {
			return true;
		}
		prev_offset = offset;
	}

	return false;
}

gb_internal Slice<i32> struct_fields_index_by_increasing_offset(gbAllocator allocator, Type *type) {
	type = base_type(type);
	GB_ASSERT(type->kind == Type_Struct);
	type_set_offsets(type);
	if (type->Struct.fields.count == 0) {
		return {};
	}
	GB_ASSERT(type->Struct.offsets != nullptr);

	auto indices = slice_make<i32>(allocator, type->Struct.fields.count);

	i64 prev_offset = 0;
	bool is_ordered = true;
	for_array(i, indices) {
		indices.data[i] = cast(i32)i;
		i64 offset = type->Struct.offsets[i];
		if (is_ordered && prev_offset > offset) {
			is_ordered = false;
		}
		prev_offset = offset;
	}
	if (!is_ordered) {
		isize n = indices.count;
		for (isize i = 1; i < n; i++) {
			isize j = i;

			while (j > 0 && type->Struct.offsets[indices[j-1]] > type->Struct.offsets[indices[j]]) {
				gb_swap(i32, indices[j-1], indices[j]);
				j -= 1;
			}
		}
	}

	return indices;
}



gb_internal i64 type_size_of(Type *t);
gb_internal i64 type_align_of(Type *t);

gb_internal i64 type_size_of_struct_pretend_is_packed(Type *ot) {
	if (ot == nullptr) {
		return 0;
	}
	Type *t = core_type(ot);
	if (t->kind != Type_Struct) {
		return type_size_of(ot);
	}

	if (t->Struct.is_packed) {
		return type_size_of(ot);
	}

	i64 count = 0, size = 0, align = 1;

	auto const &fields = t->Struct.fields;
	count = fields.count;
	if (count == 0) {
		return 0;
	}

	for_array(i, fields) {
		size += type_size_of(fields[i]->type);
	}

	return align_formula(size, align);
}


gb_internal i64 type_size_of(Type *t) {
	if (t == nullptr) {
		return 0;
	}
	i64 size = -1;
	if (t->kind == Type_Basic) {
		GB_ASSERT_MSG(is_type_typed(t), "%s", type_to_string(t));
		switch (t->Basic.kind) {
		case Basic_string:  size = 2*build_context.int_size; break;
		case Basic_cstring: size = build_context.ptr_size;   break;
		case Basic_any:     size = 16;                       break;
		case Basic_typeid:  size = 8;                        break;

		case Basic_int: case Basic_uint:
			size = build_context.int_size;
			break;
		case Basic_uintptr: case Basic_rawptr:
			size = build_context.ptr_size;
			break;
		default:
			size = t->Basic.size;
			break;
		}
		t->cached_size.store(size);
		return size;
	} else if (t->kind != Type_Named && t->cached_size >= 0) {
		return t->cached_size.load();
	} else {
		TypePath path{};
		type_path_init(&path);
		{
			MUTEX_GUARD(&g_type_mutex);
			size = type_size_of_internal(t, &path);
			t->cached_size.store(size);
		}
		type_path_free(&path);
		return size;
	}
}

gb_internal i64 type_align_of(Type *t) {
	if (t == nullptr) {
		return 1;
	}
	if (t->kind != Type_Named && t->cached_align > 0) {
		return t->cached_align.load();
	}

	TypePath path{};
	type_path_init(&path);
	{
		MUTEX_GUARD(&g_type_mutex);
		t->cached_align.store(type_align_of_internal(t, &path));
	}
	type_path_free(&path);
	return t->cached_align.load();
}


gb_internal i64 type_align_of_internal(Type *t, TypePath *path) {
	GB_ASSERT(path != nullptr);
	if (t->failure) {
		return FAILURE_ALIGNMENT;
	}

	t = base_type(t);

	switch (t->kind) {
	case Type_Basic: {
		GB_ASSERT(is_type_typed(t));
		switch (t->Basic.kind) {
		case Basic_string:  return build_context.int_size;
		case Basic_cstring: return build_context.ptr_size;
		case Basic_any:     return 8;
		case Basic_typeid:  return 8;

		case Basic_int: case Basic_uint:
			return build_context.int_size;
		case Basic_uintptr: case Basic_rawptr:
			return build_context.ptr_size;

		case Basic_complex32: case Basic_complex64: case Basic_complex128:
			return type_size_of_internal(t, path) / 2;
		case Basic_quaternion64: case Basic_quaternion128: case Basic_quaternion256:
			return type_size_of_internal(t, path) / 4;
		}
	} break;

	case Type_Array: {
		Type *elem = t->Array.elem;
		bool pop = type_path_push(path, elem);
		if (path->failure) {
			return FAILURE_ALIGNMENT;
		}
		i64 align = type_align_of_internal(elem, path);
		if (pop) type_path_pop(path);
		return align;
	}

	case Type_EnumeratedArray: {
		Type *elem = t->EnumeratedArray.elem;
		bool pop = type_path_push(path, elem);
		if (path->failure) {
			return FAILURE_ALIGNMENT;
		}
		i64 align = type_align_of_internal(elem, path);
		if (pop) type_path_pop(path);
		return align;
	}

	case Type_DynamicArray:
		// data, count, capacity, allocator
		return build_context.int_size;

	case Type_Slice:
		return build_context.int_size;

	case Type_BitField:
		return type_align_of_internal(t->BitField.backing_type, path);

	case Type_Tuple: {
		i64 max = 1;
		for_array(i, t->Tuple.variables) {
			i64 align = type_align_of_internal(t->Tuple.variables[i]->type, path);
			if (max < align) {
				max = align;
			}
		}
		return max;
	} break;

	case Type_Map:
		return build_context.ptr_size;
	case Type_Enum:
		return type_align_of_internal(t->Enum.base_type, path);

	case Type_Union: {
		if (t->Union.variants.count == 0) {
			return 1;
		}
		if (t->Union.custom_align > 0) {
			return gb_max(t->Union.custom_align, 1);
		}

		i64 max = 1;
		for_array(i, t->Union.variants) {
			Type *variant = t->Union.variants[i];
			bool pop = type_path_push(path, variant);
			if (path->failure) {
				return FAILURE_ALIGNMENT;
			}
			i64 align = type_align_of_internal(variant, path);
			if (pop) type_path_pop(path);
			if (max < align) {
				max = align;
			}
		}
		return max;
	} break;

	case Type_Struct: {
		if (t->Struct.custom_align > 0) {
			return gb_max(t->Struct.custom_align, 1);
		}

		if (t->Struct.is_packed) {
			return 1;
		}

		type_set_offsets(t);

		i64 max = 1;
		for_array(i, t->Struct.fields) {
			Type *field_type = t->Struct.fields[i]->type;
			bool pop = type_path_push(path, field_type);
			if (path->failure) {
				return FAILURE_ALIGNMENT;
			}
			i64 align = type_align_of_internal(field_type, path);
			if (pop) type_path_pop(path);
			if (max < align) {
				max = align;
			}
		}

		if (t->Struct.custom_min_field_align > 0) {
			max = gb_max(max, t->Struct.custom_min_field_align);
		}
		if (t->Struct.custom_max_field_align != 0 &&
		    t->Struct.custom_max_field_align > t->Struct.custom_min_field_align) {
			max = gb_min(max, t->Struct.custom_max_field_align);
		}
		return max;
	} break;

	case Type_BitSet: {
		if (t->BitSet.underlying != nullptr) {
			return type_align_of(t->BitSet.underlying);
		}
		i64 bits = t->BitSet.upper - t->BitSet.lower + 1;
		if (bits <= 8)   return 1;
		if (bits <= 16)  return 2;
		if (bits <= 32)  return 4;
		if (bits <= 64)  return 8;
		if (bits <= 128) return 16;
		return 8; // NOTE(bill): Could be an invalid range so limit it for now
	}

	case Type_SimdVector: {
		// IMPORTANT TODO(bill): Figure out the alignment of vector types
		return gb_clamp(next_pow2(type_size_of_internal(t, path)), 1, build_context.max_simd_align*2);
	}

	case Type_Matrix:
		return matrix_align_of(t, path);

	case Type_SoaPointer:
		return build_context.int_size;
	}

	// NOTE(bill): Things that are bigger than build_context.ptr_size, are actually comprised of smaller types
	// TODO(bill): Is this correct for 128-bit types (integers)?
	return gb_clamp(next_pow2(type_size_of_internal(t, path)), 1, build_context.max_align);
}

gb_internal i64 *type_set_offsets_of(Slice<Entity *> const &fields, bool is_packed, bool is_raw_union, i64 min_field_align, i64 max_field_align) {
	gbAllocator a = permanent_allocator();
	auto offsets = gb_alloc_array(a, i64, fields.count);
	i64 curr_offset = 0;

	if (min_field_align == 0) {
		min_field_align = 1;
	}

	TypePath path{};
	type_path_init(&path);
	defer (type_path_free(&path));

	if (is_raw_union) {
		for_array(i, fields) {
			offsets[i] = 0;
		}
	} else if (is_packed) {
		for_array(i, fields) {
			if (fields[i]->kind != Entity_Variable) {
				offsets[i] = -1;
			} else {
				i64 size = type_size_of_internal(fields[i]->type, &path);
				offsets[i] = curr_offset;
				curr_offset += size;
			}
		}
	} else {
		for_array(i, fields) {
			if (fields[i]->kind != Entity_Variable) {
				offsets[i] = -1;
			} else {
				Type *t = fields[i]->type;
				i64 align = gb_max(type_align_of_internal(t, &path), min_field_align);
				if (max_field_align > min_field_align) {
					align = gb_min(align, max_field_align);
				}
				i64 size  = gb_max(type_size_of_internal(t, &path), 0);
				curr_offset = align_formula(curr_offset, align);
				offsets[i] = curr_offset;
				curr_offset += size;
			}
		}
	}
	return offsets;
}

gb_internal bool type_set_offsets(Type *t) {
	t = base_type(t);
	if (t->kind == Type_Struct) {
		MUTEX_GUARD(&t->Struct.offset_mutex);
		if (!t->Struct.are_offsets_set) {
			t->Struct.are_offsets_being_processed = true;
			t->Struct.offsets = type_set_offsets_of(t->Struct.fields, t->Struct.is_packed, t->Struct.is_raw_union, t->Struct.custom_min_field_align, t->Struct.custom_max_field_align);
			t->Struct.are_offsets_being_processed = false;
			t->Struct.are_offsets_set = true;
			return true;
		}
	} else if (is_type_tuple(t)) {
		MUTEX_GUARD(&t->Tuple.mutex);
		if (!t->Tuple.are_offsets_set) {
			t->Tuple.are_offsets_being_processed = true;
			t->Tuple.offsets = type_set_offsets_of(t->Tuple.variables, t->Tuple.is_packed, false, 1, 0);
			t->Tuple.are_offsets_being_processed = false;
			t->Tuple.are_offsets_set = true;
			return true;
		}
	} else {
		GB_PANIC("Invalid type for setting offsets");
	}
	return false;
}

gb_internal i64 type_size_of_internal(Type *t, TypePath *path) {
	if (t->failure) {
		return FAILURE_SIZE;
	}

	switch (t->kind) {
	case Type_Named: {
		bool pop = type_path_push(path, t);
		if (path->failure) {
			return FAILURE_ALIGNMENT;
		}
		i64 size = type_size_of_internal(t->Named.base, path);
		if (pop) type_path_pop(path);
		return size;
	} break;

	case Type_Basic: {
		GB_ASSERT_MSG(is_type_typed(t), "%s", type_to_string(t));
		BasicKind kind = t->Basic.kind;
		i64 size = t->Basic.size;
		if (size > 0) {
			return size;
		}
		switch (kind) {
		case Basic_string:  return 2*build_context.int_size;
		case Basic_cstring: return build_context.ptr_size;
		case Basic_any:     return 16;
		case Basic_typeid:  return 8;

		case Basic_int: case Basic_uint:
			return build_context.int_size;
		case Basic_uintptr: case Basic_rawptr:
			return build_context.ptr_size;
		}
	} break;

	case Type_Pointer:
		return build_context.ptr_size;

	case Type_MultiPointer:
		return build_context.ptr_size;

	case Type_SoaPointer:
		return build_context.int_size*2;

	case Type_Array: {
		i64 count, align, size, alignment;
		count = t->Array.count;
		if (count == 0) {
			return 0;
		}
		align = type_align_of_internal(t->Array.elem, path);
		if (path->failure) {
			return FAILURE_SIZE;
		}
		size  = type_size_of_internal( t->Array.elem, path);
		alignment = align_formula(size, align);
		return alignment*(count-1) + size;
	} break;

	case Type_EnumeratedArray: {
		i64 count, align, size, alignment;
		count = t->EnumeratedArray.count;
		if (count == 0) {
			return 0;
		}
		align = type_align_of_internal(t->EnumeratedArray.elem, path);
		if (path->failure) {
			return FAILURE_SIZE;
		}
		size  = type_size_of_internal( t->EnumeratedArray.elem, path);
		alignment = align_formula(size, align);
		return alignment*(count-1) + size;
	} break;

	case Type_Slice: // ptr + len
		return 2 * build_context.int_size;

	case Type_DynamicArray:
		// data + len + cap + allocator(procedure+data)
		return 3*build_context.int_size + 2*build_context.ptr_size;

	case Type_Map:
		/*
			struct {
				data:      uintptr,           // 1 word
				size:      uintptr,           // 1 word
				allocator: runtime.Allocator, // 2 words
			}
		*/
		return (1 + 1 + 2)*build_context.ptr_size;

	case Type_Tuple: {
		i64 count, align, size;
		count = t->Tuple.variables.count;
		if (count == 0) {
			return 0;
		}
		align = type_align_of_internal(t, path);
		type_set_offsets(t);
		size = t->Tuple.offsets[cast(isize)count-1] + type_size_of_internal(t->Tuple.variables[cast(isize)count-1]->type, path);
		return align_formula(size, align);
	} break;

	case Type_Enum:
		return type_size_of_internal(t->Enum.base_type, path);

	case Type_Union: {
		if (t->Union.variants.count == 0) {
			return 0;
		}
		i64 align = type_align_of_internal(t, path);
		if (path->failure) {
			return FAILURE_SIZE;
		}

		i64 max = 0;

		for_array(i, t->Union.variants) {
			Type *variant_type = t->Union.variants[i];

			i64 size = type_size_of_internal(variant_type, path);
			if (max < size) {
				max = size;
			}
		}

		i64 size = 0;

		if (is_type_union_maybe_pointer(t)) {
			size = max;
			t->Union.tag_size = 0;
			t->Union.variant_block_size = size;
		} else {
			// NOTE(bill): Align to tag
			i64 tag_size = union_tag_size(t);
			size = align_formula(max, tag_size);
			// NOTE(bill): Calculate the padding between the common fields and the tag
			t->Union.tag_size = cast(i16)tag_size;
			t->Union.variant_block_size = size;

			size += tag_size;
		}
		return align_formula(size, align);
	} break;


	case Type_Struct: {
		if (t->Struct.is_raw_union) {
			i64 count = t->Struct.fields.count;
			i64 align = type_align_of_internal(t, path);
			if (path->failure) {
				return FAILURE_SIZE;
			}
			i64 max = 0;
			for (isize i = 0; i < count; i++) {
				i64 size = type_size_of_internal(t->Struct.fields[i]->type, path);
				if (max < size) {
					max = size;
				}
			}
			return align_formula(max, align);
		} else {
			i64 count = 0, size = 0, align = 0;

			count = t->Struct.fields.count;
			if (count == 0) {
				return 0;
			}
			align = type_align_of_internal(t, path);
			if (path->failure) {
				return FAILURE_SIZE;
			}
			if (t->Struct.are_offsets_being_processed && t->Struct.offsets == nullptr) {
				type_path_print_illegal_cycle(path, path->path.count-1);
				return FAILURE_SIZE;
			}
			type_set_offsets(t);
			GB_ASSERT(t->Struct.fields.count == 0 || t->Struct.offsets != nullptr);
			size = t->Struct.offsets[cast(isize)count-1] + type_size_of_internal(t->Struct.fields[cast(isize)count-1]->type, path);
			return align_formula(size, align);
		}
	} break;

	case Type_BitSet: {
		if (t->BitSet.underlying != nullptr) {
			return type_size_of(t->BitSet.underlying);
		}
		i64 bits = t->BitSet.upper - t->BitSet.lower + 1;
		if (bits <= 8)   return 1;
		if (bits <= 16)  return 2;
		if (bits <= 32)  return 4;
		if (bits <= 64)  return 8;
		if (bits <= 128) return 16;
		return 8; // NOTE(bill): Could be an invalid range so limit it for now
	}

	case Type_SimdVector: {
		i64 count = t->SimdVector.count;
		Type *elem = t->SimdVector.elem;
		return count * type_size_of_internal(elem, path);
	}

	case Type_Matrix: {
		i64 stride_in_bytes = matrix_type_stride_in_bytes(t, path);
		if (t->Matrix.is_row_major) {
			return stride_in_bytes * t->Matrix.row_count;
		} else {
			return stride_in_bytes * t->Matrix.column_count;
		}
	}

	case Type_BitField:
		return type_size_of_internal(t->BitField.backing_type, path);
	}

	// Catch all
	return build_context.ptr_size;
}

gb_internal i64 type_offset_of(Type *t, i64 index, Type **field_type_) {
	t = base_type(t);
	switch (t->kind) {
	case Type_Struct:
		type_set_offsets(t);
		if (gb_is_between(index, 0, t->Struct.fields.count-1)) {
			GB_ASSERT(t->Struct.offsets != nullptr);
			if (field_type_) *field_type_ = t->Struct.fields[index]->type;
			return t->Struct.offsets[index];
		}
		break;
	case Type_Tuple:
		type_set_offsets(t);
		if (gb_is_between(index, 0, t->Tuple.variables.count-1)) {
			GB_ASSERT(t->Tuple.offsets != nullptr);
			if (field_type_) *field_type_ = t->Tuple.variables[index]->type;
			i64 offset = t->Tuple.offsets[index];
			GB_ASSERT(offset >= 0);
			return offset;
		}
		break;

	case Type_Array:
		GB_ASSERT(0 <= index && index < t->Array.count);
		return index * type_size_of(t->Array.elem);

	case Type_Basic:
		if (t->Basic.kind == Basic_string) {
			switch (index) {
			case 0:
				if (field_type_) *field_type_ = t_u8_ptr;
				return 0;                      // data
			case 1:
				if (field_type_) *field_type_ = t_int;
				return build_context.int_size; // len
			}
		} else if (t->Basic.kind == Basic_any) {
			switch (index) {
			case 0:
				if (field_type_) *field_type_ = t_rawptr;
				return 0;                      // data
			case 1:
				if (field_type_) *field_type_ = t_typeid;
				return 8; // id
			}
		}
		break;
	case Type_Slice:
		switch (index) {
		case 0:
			if (field_type_) *field_type_ = alloc_type_multi_pointer(t->Slice.elem);
			return 0;                        // data
		case 1:
			if (field_type_) *field_type_ = t_int;
			return 1*build_context.int_size; // len
		}
		break;
	case Type_DynamicArray:
		switch (index) {
		case 0:
			if (field_type_) *field_type_ = alloc_type_multi_pointer(t->DynamicArray.elem);
			return 0;                        // data
		case 1:
			if (field_type_) *field_type_ = t_int;
			return 1*build_context.int_size; // len
		case 2:
			if (field_type_) *field_type_ = t_int;
			return 2*build_context.int_size; // cap
		case 3:
			if (field_type_) *field_type_ = t_allocator;
			return 3*build_context.int_size; // allocator
		}
		break;
	case Type_Union:
		if (!is_type_union_maybe_pointer(t)) {
			/* i64 s = */ type_size_of(t);
			switch (index) {
			case -1:
				if (field_type_) *field_type_ = union_tag_type(t);
				union_tag_size(t);
				return t->Union.variant_block_size;
			}
		}
		break;
	}
	GB_ASSERT(index == 0);
	return 0;
}


gb_internal i64 type_offset_of_from_selection(Type *type, Selection sel) {
	GB_ASSERT(sel.indirect == false);

	Type *t = type;
	i64 offset = 0;
	for_array(i, sel.index) {
		i32 index = sel.index[i];
		t = base_type(t);
		offset += type_offset_of(t, index);
		if (t->kind == Type_Struct) {
			t = t->Struct.fields[index]->type;
		} else if (t->kind == Type_Array) {
			t = t->Array.elem;
		} else {
			// NOTE(bill): No need to worry about custom types, just need the alignment
			switch (t->kind) {
			case Type_Basic:
				if (t->Basic.kind == Basic_string) {
					switch (index) {
					case 0: t = t_rawptr; break;
					case 1: t = t_int;    break;
					}
				} else if (t->Basic.kind == Basic_any) {
					switch (index) {
					case 0: t = t_rawptr; break;
					case 1: t = t_typeid; break;
					}
				}
				break;
			case Type_Slice:
				switch (index) {
				case 0: t = t_rawptr; break;
				case 1: t = t_int;    break;
				case 2: t = t_int;    break;
				}
				break;
			case Type_DynamicArray:
				switch (index) {
				case 0: t = t_rawptr;    break;
				case 1: t = t_int;       break;
				case 2: t = t_int;       break;
				case 3: t = t_allocator; break;
				}
				break;
			}
		}
	}
	return offset;
}

gb_internal isize check_is_assignable_to_using_subtype(Type *src, Type *dst, isize level = 0, bool src_is_ptr = false, bool allow_polymorphic=false) {
	Type *prev_src = src;
	src = type_deref(src);
	if (!src_is_ptr) {
		src_is_ptr = src != prev_src;
	}
	src = base_type(src);

	if (!is_type_struct(src)) {
		return 0;
	}

	bool dst_is_polymorphic = is_type_polymorphic(dst);

	for_array(i, src->Struct.fields) {
		Entity *f = src->Struct.fields[i];
		if (f->kind != Entity_Variable || (f->flags&EntityFlags_IsSubtype) == 0) {
			continue;
		}
		if (allow_polymorphic && dst_is_polymorphic) {
			Type *fb = base_type(type_deref(f->type));
			if (fb->kind == Type_Struct) {
				if (fb->Struct.polymorphic_parent == dst) {
					return true;
				}
			}
		}

		if (are_types_identical(f->type, dst)) {
			return level+1;
		}
		if (src_is_ptr && is_type_pointer(dst)) {
			if (are_types_identical(f->type, type_deref(dst))) {
				return level+1;
			}
		}
		isize nested_level = check_is_assignable_to_using_subtype(f->type, dst, level+1, src_is_ptr, allow_polymorphic);
		if (nested_level > 0) {
			return nested_level;
		}
	}

	return 0;
}

gb_internal bool is_type_subtype_of(Type *src, Type *dst) {
	if (are_types_identical(src, dst)) {
		return true;
	}

	return 0 < check_is_assignable_to_using_subtype(src, dst, 0, is_type_pointer(src));
}
gb_internal bool is_type_subtype_of_and_allow_polymorphic(Type *src, Type *dst) {
	if (are_types_identical(src, dst)) {
		return true;
	}

	return 0 < check_is_assignable_to_using_subtype(src, dst, 0, is_type_pointer(src), true);
}


gb_internal bool has_type_got_objc_class_attribute(Type *t) {
	return t->kind == Type_Named && t->Named.type_name != nullptr && t->Named.type_name->TypeName.objc_class_name != "";
}



gb_internal bool internal_check_is_assignable_to(Type *src, Type *dst);
gb_internal bool is_type_objc_object(Type *t) {
	return internal_check_is_assignable_to(t, t_objc_object);
}

gb_internal Type *get_struct_field_type(Type *t, isize index) {
	t = base_type(type_deref(t));
	GB_ASSERT(t->kind == Type_Struct);
	return t->Struct.fields[index]->type;
}


gb_internal Type *reduce_tuple_to_single_type(Type *original_type) {
	if (original_type != nullptr) {
		Type *t = core_type(original_type);
		if (t->kind == Type_Tuple && t->Tuple.variables.count == 1) {
			return t->Tuple.variables[0]->type;
		}
	}
	return original_type;
}

gb_internal Type *alloc_type_tuple_from_field_types(Type **field_types, isize field_count, bool is_packed, bool must_be_tuple) {
	if (field_count == 0) {
		return nullptr;
	}
	if (!must_be_tuple && field_count == 1) {
		return field_types[0];
	}

	Type *t = alloc_type_tuple();
	t->Tuple.variables = slice_make<Entity *>(heap_allocator(), field_count);

	Scope *scope = nullptr;
	for_array(i, t->Tuple.variables) {
		t->Tuple.variables[i] = alloc_entity_param(scope, blank_token, field_types[i], false, false);
	}
	t->Tuple.is_packed = is_packed;

	return t;
}

gb_internal Type *alloc_type_proc_from_types(Type **param_types, unsigned param_count, Type *results, bool is_c_vararg, ProcCallingConvention calling_convention) {

	Type *params  = alloc_type_tuple_from_field_types(param_types, param_count, false, true);
	isize results_count = 0;
	if (results != nullptr) {
		if (results->kind != Type_Tuple) {
			results = alloc_type_tuple_from_field_types(&results, 1, false, true);
		}
		results_count = results->Tuple.variables.count;
	}

	Scope *scope = nullptr;
	Type *t = alloc_type_proc(scope, params, param_count, results, results_count, false, calling_convention);
	t->Proc.c_vararg = is_c_vararg;
	return t;
}

// gb_internal Type *type_from_selection(Type *type, Selection const &sel) {
// 	for (i32 index : sel.index) {
// 		Type *bt = base_type(type_deref(type));
// 		switch (bt->kind) {
// 		case Type_Struct:
// 			type = bt->Struct.fields[index]->type;
// 			break;
// 		case Type_Tuple:
// 			type = bt->Tuple.variables[index]->type;
// 			break;
// 		case Type_BitField:
// 			type = bt->BitField.fields[index]->type;
// 			break;
// 		case Type_Array:
// 			type = bt->Array.elem;
// 			break;
// 		case Type_EnumeratedArray:
// 			type = bt->Array.elem;
// 			break;
// 		case Type_Slice:
// 			switch (index) {
// 			case 0: type = alloc_type_multi_pointer(bt->Slice.elem); break;
// 			case 1: type = t_int;                                    break;
// 			}
// 			break;
// 		case Type_DynamicArray:
// 			switch (index) {
// 			case 0: type = alloc_type_multi_pointer(bt->DynamicArray.elem); break;
// 			case 1: type = t_int;                                           break;
// 			case 2: type = t_int;                                           break;
// 			case 3: type = t_allocator;                                     break;
// 			}
// 			break;
// 		case Type_Map:
// 			switch (index) {
// 			case 0: type = t_uintptr;   break;
// 			case 1: type = t_int;       break;
// 			case 2: type = t_allocator; break;
// 			}
// 			break;
// 		case Type_Basic:
// 			if (is_type_complex_or_quaternion(bt)) {
// 				type = base_complex_elem_type(bt);
// 			} else {
// 				switch (type->Basic.kind) {
// 				case Basic_any:
// 					switch (index) {
// 					case 0: type = t_rawptr; break;
// 					case 1: type = t_typeid; break;
// 					}
// 					break;
// 				case Basic_string:
// 					switch (index) {
// 					case 0: type = t_u8_multi_ptr; break;
// 					case 1: type = t_int;          break;
// 					}
// 					break;
// 				}
// 			}
// 			break;
// 		}
// 	}
// 	return type;
// }

// Index a type that is internally a struct or array.
gb_internal Type *type_internal_index(Type *t, isize index) {
	Type *bt = base_type(t);
	if (bt == nullptr) {
		return nullptr;
	}

	switch (bt->kind) {
	case Type_Basic:
		{
			switch (bt->Basic.kind) {
			case Basic_complex32:     return t_f16;
			case Basic_complex64:     return t_f32;
			case Basic_complex128:    return t_f64;
			case Basic_quaternion64:  return t_f16;
			case Basic_quaternion128: return t_f32;
			case Basic_quaternion256: return t_f64;
			case Basic_string:
				{
					GB_ASSERT(index == 0 || index == 1);
					return index == 0 ? t_u8_ptr : t_int;
				}
			case Basic_any:
				{
					GB_ASSERT(index == 0 || index == 1);
					return index == 0 ? t_rawptr : t_typeid;
				}
			}
		}
		break;

	case Type_Array:           return bt->Array.elem;
	case Type_EnumeratedArray: return bt->EnumeratedArray.elem;
	case Type_SimdVector:      return bt->SimdVector.elem;
	case Type_Slice:
		{
			GB_ASSERT(index == 0 || index == 1);
			return index == 0 ? t_rawptr : t_typeid;
		}
	case Type_DynamicArray:
		{
			switch (index) {
			case 0:  return t_rawptr;
			case 1:  return t_int;
			case 2:  return t_int;
			case 3:  return t_allocator;
			default: GB_PANIC("invalid raw dynamic array index");
			};
		}
	case Type_Struct:
		return get_struct_field_type(bt, index);
	case Type_Union:
		if (index < bt->Union.variants.count) {
			return bt->Union.variants[index];
		}
		return union_tag_type(bt);
	case Type_Tuple:
		return bt->Tuple.variables[index]->type;
	case Type_Matrix:
		return bt->Matrix.elem;
	case Type_SoaPointer:
		{
			GB_ASSERT(index == 0 || index == 1);
			return index == 0 ? t_rawptr : t_int;
		}
	case Type_Map:
		return type_internal_index(bt->Map.debug_metadata_type, index);
	case Type_BitField:
		return type_internal_index(bt->BitField.backing_type, index);
	case Type_Generic:
		return type_internal_index(bt->Generic.specialized, index);
	};

	GB_PANIC("Unhandled type %s", type_to_string(bt));
	return nullptr;
};

gb_internal gbString write_type_to_string(gbString str, Type *type, bool shorthand=false, bool allow_polymorphic=false) {
	if (type == nullptr) {
		return gb_string_appendc(str, "<no type>");
	}

	switch (type->kind) {
	case Type_Basic:
		str = gb_string_append_length(str, type->Basic.name.text, type->Basic.name.len);
		break;

	case Type_Generic:
		if (type->Generic.name.len == 0) {
			if (type->Generic.entity != nullptr) {
				String name = type->Generic.entity->token.string;
				str = gb_string_append_rune(str, '$');
				str = gb_string_append_length(str, name.text, name.len);
			} else {
				str = gb_string_appendc(str, "type");
			}
		} else {
			String name = type->Generic.name;
			str = gb_string_append_rune(str, '$');
			str = gb_string_append_length(str, name.text, name.len);
			if (type->Generic.specialized != nullptr) {
				str = gb_string_append_rune(str, '/');
				str = write_type_to_string(str, type->Generic.specialized, shorthand, allow_polymorphic);
			}
		}
		break;

	case Type_Pointer:
		str = gb_string_append_rune(str, '^');
		str = write_type_to_string(str, type->Pointer.elem, shorthand, allow_polymorphic);
		break;

	case Type_SoaPointer:
		str = gb_string_appendc(str, "#soa ^");
		str = write_type_to_string(str, type->SoaPointer.elem, shorthand, allow_polymorphic);
		break;

	case Type_MultiPointer:
		str = gb_string_appendc(str, "[^]");
		str = write_type_to_string(str, type->Pointer.elem, shorthand, allow_polymorphic);
		break;

	case Type_EnumeratedArray:
		if (type->EnumeratedArray.is_sparse) {
			str = gb_string_appendc(str, "#sparse");
		}
		str = gb_string_append_rune(str, '[');
		str = write_type_to_string(str, type->EnumeratedArray.index, shorthand, allow_polymorphic);
		str = gb_string_append_rune(str, ']');
		str = write_type_to_string(str, type->EnumeratedArray.elem, shorthand, allow_polymorphic);
		break;

	case Type_Array:
		str = gb_string_appendc(str, gb_bprintf("[%lld]", cast(long long)type->Array.count));
		str = write_type_to_string(str, type->Array.elem, shorthand, allow_polymorphic);
		break;

	case Type_Slice:
		str = gb_string_appendc(str, "[]");
		str = write_type_to_string(str, type->Array.elem, shorthand, allow_polymorphic);
		break;

	case Type_DynamicArray:
		str = gb_string_appendc(str, "[dynamic]");
		str = write_type_to_string(str, type->DynamicArray.elem, shorthand, allow_polymorphic);
		break;

	case Type_Enum:
		str = gb_string_appendc(str, "enum");
		if (type->Enum.base_type != nullptr) {
			str = gb_string_appendc(str, " ");
			str = write_type_to_string(str, type->Enum.base_type, shorthand, allow_polymorphic);
		}
		str = gb_string_appendc(str, " {");
		for_array(i, type->Enum.fields) {
			Entity *f = type->Enum.fields[i];
			GB_ASSERT(f->kind == Entity_Constant);
			if (i > 0) {
				str = gb_string_appendc(str, ", ");
			}
			str = gb_string_append_length(str, f->token.string.text, f->token.string.len);
			// str = gb_string_appendc(str, " = ");
		}
		str = gb_string_append_rune(str, '}');
		break;

	case Type_Union:
		str = gb_string_appendc(str, "union");

		if (allow_polymorphic && type->Struct.polymorphic_params) {
			str = gb_string_appendc(str, "(");
			str = write_type_to_string(str, type->Struct.polymorphic_params, shorthand, allow_polymorphic);
			str = gb_string_appendc(str, ")");
		}

		switch (type->Union.kind) {
		case UnionType_no_nil:     str = gb_string_appendc(str, " #no_nil");     break;
		case UnionType_shared_nil: str = gb_string_appendc(str, " #shared_nil"); break;
		}
		if (type->Union.custom_align != 0) str = gb_string_append_fmt(str, " #align %d", cast(int)type->Union.custom_align);
		str = gb_string_appendc(str, " {");
		for_array(i, type->Union.variants) {
			Type *t = type->Union.variants[i];
			if (i > 0) str = gb_string_appendc(str, ", ");
			str = write_type_to_string(str, t, shorthand, allow_polymorphic);
		}
		str = gb_string_append_rune(str, '}');
		break;

	case Type_Struct: {
		if (type->Struct.soa_kind != StructSoa_None) {
			switch (type->Struct.soa_kind) {
			case StructSoa_Fixed:   str = gb_string_append_fmt(str, "#soa[%d]", cast(int)type->Struct.soa_count); break;
			case StructSoa_Slice:   str = gb_string_appendc(str,    "#soa[]");                                    break;
			case StructSoa_Dynamic: str = gb_string_appendc(str,    "#soa[dynamic]");                             break;
			default: GB_PANIC("Unknown StructSoaKind"); break;
			}
			str = write_type_to_string(str, type->Struct.soa_elem, shorthand, allow_polymorphic);
			break;
		}

		str = gb_string_appendc(str, "struct");

		if (allow_polymorphic && type->Struct.polymorphic_params) {
			str = gb_string_appendc(str, "(");
			str = write_type_to_string(str, type->Struct.polymorphic_params, shorthand, allow_polymorphic);
			str = gb_string_appendc(str, ")");
		}

		if (type->Struct.is_packed)    str = gb_string_appendc(str, " #packed");
		if (type->Struct.is_raw_union) str = gb_string_appendc(str, " #raw_union");
		if (type->Struct.custom_align != 0) str = gb_string_append_fmt(str, " #align %d", cast(int)type->Struct.custom_align);

		str = gb_string_appendc(str, " {");

		if (shorthand && type->Struct.fields.count > 16) {
			str = gb_string_append_fmt(str, "%lld fields...", cast(long long)type->Struct.fields.count);
		} else {
			for_array(i, type->Struct.fields) {
				Entity *f = type->Struct.fields[i];
				GB_ASSERT(f->kind == Entity_Variable);
				if (i > 0) {
					str = gb_string_appendc(str, ", ");
				}
				str = gb_string_append_length(str, f->token.string.text, f->token.string.len);
				str = gb_string_appendc(str, ": ");
				str = write_type_to_string(str, f->type, shorthand, allow_polymorphic);
			}
		}
		str = gb_string_append_rune(str, '}');
	} break;

	case Type_Map: {
		str = gb_string_appendc(str, "map[");
		str = write_type_to_string(str, type->Map.key, shorthand, allow_polymorphic);
		str = gb_string_append_rune(str, ']');
		str = write_type_to_string(str, type->Map.value, shorthand, allow_polymorphic);
	} break;

	case Type_Named:
		if (type->Named.type_name != nullptr) {
			str = gb_string_append_length(str, type->Named.name.text, type->Named.name.len);
		} else {
			// NOTE(bill): Just in case
			str = gb_string_appendc(str, "<named type>");
		}
		break;

	case Type_Tuple:
		if (type->Tuple.variables.count > 0) {
			isize comma_index = 0;
			for_array(i, type->Tuple.variables) {
				Entity *var = type->Tuple.variables[i];
				if (var == nullptr) {
					continue;
				}
				if (comma_index++ > 0) {
					str = gb_string_appendc(str, ", ");
				}

				String name = var->token.string;
				if (var->kind == Entity_Constant) {
					str = gb_string_appendc(str, "$");
					str = gb_string_append_length(str, name.text, name.len);
					if (!is_type_untyped(var->type)) {
						str = gb_string_appendc(str, ": ");
						str = write_type_to_string(str, var->type, shorthand, allow_polymorphic);
						if (var->Constant.value.kind) {
							str = gb_string_appendc(str, " = ");
							str = write_exact_value_to_string(str, var->Constant.value);
						}
					} else {
						str = gb_string_appendc(str, " := ");
						str = write_exact_value_to_string(str, var->Constant.value);
					}
					continue;
				}

				if (var->kind == Entity_Variable) {
					if (var->flags&EntityFlag_CVarArg) {
						str = gb_string_appendc(str, "#c_vararg ");
					}
					if (var->flags&EntityFlag_Ellipsis) {
						Type *slice = base_type(var->type);
						str = gb_string_appendc(str, "..");
						GB_ASSERT(var->type->kind == Type_Slice);
						str = write_type_to_string(str, slice->Slice.elem, shorthand, allow_polymorphic);
					} else {
						str = write_type_to_string(str, var->type, shorthand, allow_polymorphic);
					}
				} else {
					GB_ASSERT(var->kind == Entity_TypeName);
					if (var->type->kind == Type_Generic) {
						if (var->token.string.len != 0) {
							String name = var->token.string;
							str = gb_string_appendc(str, "$");
							str = gb_string_append_length(str, name.text, name.len);
							str = gb_string_appendc(str, ": typeid");
							if (var->type->Generic.specialized) {
								str = gb_string_appendc(str, "/");
								str = write_type_to_string(str, var->type->Generic.specialized, shorthand, allow_polymorphic);
							}
						} else {
							str = gb_string_appendc(str, "typeid/");
							str = write_type_to_string(str, var->type, shorthand, allow_polymorphic);
						}
					} else {
						str = gb_string_appendc(str, "$");
						str = gb_string_append_length(str, name.text, name.len);
						str = gb_string_appendc(str, "=");
						str = write_type_to_string(str, var->type, shorthand, allow_polymorphic);
					}
				}
			}
		}
		break;

	case Type_Proc:
		str = gb_string_appendc(str, "proc");

		switch (type->Proc.calling_convention) {
		case ProcCC_Odin:
			if (default_calling_convention() != ProcCC_Odin) {
				str = gb_string_appendc(str, " \"odin\" ");
			}
			break;
		case ProcCC_Contextless:
			if (default_calling_convention() != ProcCC_Contextless) {
				str = gb_string_appendc(str, " \"contextless\" ");
			}
			break;
		case ProcCC_CDecl:
			str = gb_string_appendc(str, " \"c\" ");
			break;
		case ProcCC_StdCall:
			str = gb_string_appendc(str, " \"std\" ");
			break;
		case ProcCC_FastCall:
			str = gb_string_appendc(str, " \"fastcall\" ");
			break;
			break;
		case ProcCC_None:
			str = gb_string_appendc(str, " \"none\" ");
			break;
		case ProcCC_Naked:
			str = gb_string_appendc(str, " \"naked\" ");
			break;
		// case ProcCC_VectorCall:
		// 	str = gb_string_appendc(str, " \"vectorcall\" ");
		// 	break;
		// case ProcCC_ClrCall:
		// 	str = gb_string_appendc(str, " \"clrcall\" ");
		// 	break;
		}
		str = gb_string_appendc(str, "(");
		if (type->Proc.params) {
			str = write_type_to_string(str, type->Proc.params, shorthand, allow_polymorphic);
		}
		str = gb_string_appendc(str, ")");
		if (type->Proc.results) {
			str = gb_string_appendc(str, " -> ");
			if (type->Proc.results->Tuple.variables.count > 1) {
				str = gb_string_appendc(str, "(");
			}
			str = write_type_to_string(str, type->Proc.results, shorthand, allow_polymorphic);
			if (type->Proc.results->Tuple.variables.count > 1) {
				str = gb_string_appendc(str, ")");
			}
		}
		break;

	case Type_BitSet:
		str = gb_string_appendc(str, "bit_set[");
		if (type->BitSet.elem == nullptr) {
			str = gb_string_appendc(str, "<unresolved>");
		} else if (is_type_enum(type->BitSet.elem)) {
			str = write_type_to_string(str, type->BitSet.elem, shorthand, allow_polymorphic);
		} else {
			str = gb_string_append_fmt(str, "%lld", type->BitSet.lower);
			str = gb_string_append_fmt(str, "..=");
			str = gb_string_append_fmt(str, "%lld", type->BitSet.upper);
		}
		if (type->BitSet.underlying != nullptr) {
			str = gb_string_appendc(str, "; ");
			str = write_type_to_string(str, type->BitSet.underlying, shorthand, allow_polymorphic);
		}
		str = gb_string_appendc(str, "]");
		break;

	case Type_SimdVector:
		str = gb_string_append_fmt(str, "#simd[%d]", cast(int)type->SimdVector.count);
		str = write_type_to_string(str, type->SimdVector.elem, shorthand, allow_polymorphic);
		break;

	case Type_Matrix:
		if (type->Matrix.is_row_major) {
			str = gb_string_appendc(str, "#row_major ");
		}
		str = gb_string_appendc(str, gb_bprintf("matrix[%d, %d]", cast(int)type->Matrix.row_count, cast(int)type->Matrix.column_count));
		str = write_type_to_string(str, type->Matrix.elem, shorthand, allow_polymorphic);
		break;

	case Type_BitField:
		str = gb_string_appendc(str, "bit_field ");
		str = write_type_to_string(str, type->BitField.backing_type, shorthand, allow_polymorphic);
		str = gb_string_appendc(str, " {");
		for (isize i = 0; i < type->BitField.fields.count; i++) {
			Entity *f = type->BitField.fields[i];
			if (i > 0) {
				str = gb_string_appendc(str, ", ");
			}
			str = gb_string_append_length(str, f->token.string.text, f->token.string.len);
			str = gb_string_appendc(str, ": ");
			str = write_type_to_string(str, f->type, shorthand, allow_polymorphic);
			str = gb_string_append_fmt(str, " | %u", type->BitField.bit_sizes[i]);
		}
		str = gb_string_appendc(str, " }");
		break;
	}

	return str;
}


gb_internal gbString type_to_string(Type *type, gbAllocator allocator, bool shorthand) {
	return write_type_to_string(gb_string_make(allocator, ""), type, shorthand);
}
gb_internal gbString type_to_string(Type *type, bool shorthand) {
	return write_type_to_string(gb_string_make(heap_allocator(), ""), type, shorthand);
}

gb_internal gbString type_to_string_polymorphic(Type *type) {
	return write_type_to_string(gb_string_make(heap_allocator(), ""), type, false, true);
}


gb_internal gbString type_to_string_shorthand(Type *type) {
	return type_to_string(type, true);
}
