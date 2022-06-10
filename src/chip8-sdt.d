provider chip8 
{
  probe exec_begin ();
  probe illegal_opcode (unsigned short opcode);
  probe exec_end ();
};

//struct astruct {int a; int b;};
