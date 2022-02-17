namespace mscript {

using Instruction = std::uint16_t;

namespace InstrCode {

//Note stack arguments are shown from top to below

///No operation
static const Instruction noop = 0;
///Discard (value) -> nothing   - deletes one item from stack
static const Instruction discard = 1;
///Duplicates value   (value)->(value,value)
static const Instruction dup = 2;
///Insert unsigned number 2 bytes
static const Instruction insert_int_2 =3;
///Insert unsigned number 4 bytes
static const Instruction insert_int_4 =4;
///Insert unsigned number 8 bytes
static const Instruction insert_int_8 =5;
///Insert double 8 bytes
static const Instruction insert_double =6;
///Reset IR register
static const Instruction reset_ir = 7;
///Load IR register from stack
static const Instruction load_ir = 8;
///Load IR register from code
static const Instruction load_ir_2 = 9;
///Load IR register from code
static const Instruction load_ir_4 = 10;
///Load IR register from code
static const Instruction load_ir_8 = 11;
///Assign content of stack to one variable (value, variable)->(value)
static const Instruction assign1 = 12;
///Multiassign, assign from array, indexed by IR register, each attempt increases register
/** (value,variabl) -> (value)  IR+ */
static const Instruction assignN = 13;
///Pack N results from stacks to single array - IR register contains count of elements. TOP result is last
static const Instruction packN = 14;
///Object index  (name, value) -> value
static const Instruction object_deref= 15;
///Variable dereference (name)->value
static const Instruction variable_deref= 16;
///Insert IR to stack
static const Instruction insert_ir = 17;
///Reset loop register
static const Instruction reset_lr = 18;
///Load item from array indexed by LR, increases LR. Sets flag, if there is no more items
static const Instruction load_at_lr = 19;
///Jump at index when flag is set (index is absolute in current block)
static const Instruction jump_iff = 20;
///Jump at index when flag is not set (index is absolute in current block)
static const Instruction jump_ifnf = 21;


}


}
