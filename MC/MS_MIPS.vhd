-- ECE 3056: Architecture, Concurrency and Energy in Computation
-- Sudhakar Yalamanchili
-- Multicycle MIPS Processor VHDL Behavioral Model
--				
-- Top Level Structural Model for MIPS Processor Core
--
-- School of Electrical & Computer Engineering
-- Georgia Institute of Technology
-- Atlanta, GA 30332

-- Sho Ko
-- 903197992
-- Declare all the input, output, and intermediate signals added in other files

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_ARITH.ALL;

ENTITY MIPS IS -- empty entity since clocks are generated in the model
END 	MIPS;
									  
ARCHITECTURE structure OF MIPS IS

-- declare all of the components to be used in this module

COMPONENT my_clock
	  PORT (sys_clock, reset : OUT STD_LOGIC);
	  END COMPONENT;
	        
COMPONENT Ifetch
	PORT(	-- input signals 
                PC_Plus_4	          	: IN 	STD_LOGIC_VECTOR( 31 DOWNTO 0 );
        	Branch_PC               	: IN 	STD_LOGIC_VECTOR( 31 DOWNTO 0 );
                Jump_PC                         : IN    STD_LOGIC_VECTOR( 31 DOWNTO 0 ); -- input Jump_PC
                Jr_PC                           : IN    STD_LOGIC_VECTOR( 31 DOWNTO 0 ); -- input Jr_PC
        	PCWrite, PCWriteCond, IRWrite   : IN STD_LOGIC;
        	PCSource 			: IN STD_LOGIC_VECTOR(1 DOWNTO 0);
        	Zero, MemRead, MemWrite         : IN STD_LOGIC;
        	clock, reset, IorD	        : IN STD_LOGIC;
        	memory_data_in   	        : IN std_logic_vector(31 downto 0);
        	memory_addr_in   	        : IN std_logic_vector(31 downto 0);
        	
		-- Output signals

        	PC_out 			  : OUT STD_LOGIC_VECTOR( 31 DOWNTO 0 );
        	Memory_data_out 	  : OUT STD_LOGIC_VECTOR( 31 DOWNTO 0 );
        	Instruction 		  : OUT STD_LOGIC_VECTOR( 31 DOWNTO 0 )); -- output instruction
END COMPONENT Ifetch;

COMPONENT Idecode	
	PORT(	read_data_1	: OUT STD_LOGIC_VECTOR( 31 DOWNTO 0 );
		read_data_2	: OUT STD_LOGIC_VECTOR( 31 DOWNTO 0 );
		Sign_extend     : OUT STD_LOGIC_VECTOR( 31 DOWNTO 0 );

		Instruction     : IN STD_LOGIC_VECTOR( 31 DOWNTO 0 );
		read_data 	: IN STD_LOGIC_VECTOR( 31 DOWNTO 0 );
		ALU_result	: IN STD_LOGIC_VECTOR( 31 DOWNTO 0 );
		RegWrite  	: IN STD_LOGIC;
		MemtoReg 	: IN STD_LOGIC;
		RegDst 	        : IN STD_LOGIC;
		clock,reset	: IN STD_LOGIC );
END COMPONENT Idecode;


COMPONENT control
	PORT( 
		Opcode 		: IN 	STD_LOGIC_VECTOR( 5 DOWNTO 0 ); -- input opcode
                Function_opcode : IN    STD_LOGIC_VECTOR( 5 downto 0 ); -- input function opcode
		clock, reset	: IN 	STD_LOGIC;
	 
		PCWrite         : OUT STD_LOGIC;
   		PCWriteCond     : OUT STD_LOGIC;
   		IorD            : OUT STD_LOGIC;
   		MemRead 	: OUT STD_LOGIC;
   		MemWrite 	: OUT STD_LOGIC;
  		IRWrite         : OUT STD_LOGIC;
   		MemtoReg        : OUT STD_LOGIC;
   		PCSource        : OUT STD_LOGIC_VECTOR (1 DOWNTO 0);
   		ALUOp           : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
   		ALUSrcB 	: OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
   		ALUSrcA         : OUT STD_LOGIC;
   		RegWrite 	: OUT STD_LOGIC;
   		RegDst          : OUT STD_LOGIC;
   		micropc         : out integer);

END COMPONENT control;


COMPONENT  Execute	
	PORT(	Read_data_1 	   : IN STD_LOGIC_VECTOR( 31 DOWNTO 0 );
		Read_data_2 	   : IN STD_LOGIC_VECTOR( 31 DOWNTO 0 );
		Sign_extend 	   : IN STD_LOGIC_VECTOR( 31 DOWNTO 0 );
                Instruction        : IN STD_LOGIC_VECTOR( 31 DOWNTO 0 ); -- input instruction
		Function_opcode    : IN STD_LOGIC_VECTOR( 5 DOWNTO 0 ); -- input function opcode
		ALUOp 		   : IN STD_LOGIC_VECTOR( 1 DOWNTO 0 );
		ALUSrcB		   : IN STD_LOGIC_VECTOR( 1 DOWNTO 0 );
		ALUSrcA 	   : IN STD_LOGIC;
		PC 	           : IN STD_LOGIC_VECTOR( 31 DOWNTO 0 );
		clock, reset	   : IN STD_LOGIC;
			
		Zero 	      	   : OUT STD_LOGIC;
		ALU_Result     	   : OUT STD_LOGIC_VECTOR( 31 DOWNTO 0 );
		ALUOut 		   : OUT STD_LOGIC_VECTOR( 31 DOWNTO 0 );
                Jr_Address      : OUT   STD_LOGIC_VECTOR( 31 DOWNTO 0 ); -- output jr address
                Jump_Address       : OUT   STD_LOGIC_VECTOR( 31 DOWNTO 0 )); -- output jump address
END COMPONENT Execute;

					-- declare signals used to connect VHDL components
	SIGNAL s_PC_Out   		: STD_LOGIC_VECTOR( 31 DOWNTO 0 ); 
	SIGNAL s_Instruction		: STD_LOGIC_VECTOR( 31 DOWNTO 0 ); -- intermediate instruction
	SIGNAL s_read_data_1 		: STD_LOGIC_VECTOR( 31 DOWNTO 0 );
	SIGNAL s_read_data_2 		: STD_LOGIC_VECTOR( 31 DOWNTO 0 );
	SIGNAL s_memory_data_out 	: STD_LOGIC_VECTOR( 31 DOWNTO 0 );
	SIGNAL s_Sign_Extend 		: STD_LOGIC_VECTOR( 31 DOWNTO 0 );
	SIGNAL s_ALU_result 		: STD_LOGIC_VECTOR( 31 DOWNTO 0 );
	SIGNAL s_ALUOut        	   	: STD_LOGIC_VECTOR( 31 DOWNTO 0 );
	SIGNAL s_ALUSrcB      		: STD_LOGIC_vector(1 downto 0);
	SIGNAL s_ALUop 			: STD_LOGIC_VECTOR(1 DOWNTO 0 );
	SIGNAL s_PCSource    		: STD_LOGIC_VECTOR (1 DOWNTO 0);
        SIGNAL s_Jump_Address           : STD_LOGIC_VECTOR( 31 DOWNTO 0 ); -- intermediate jump address
        SIGNAL s_Jr_Address             : STD_LOGIC_VECTOR( 31 DOWNTO 0 ); -- intermediate jr address
	
        SIGNAL s_reset, s_sys_clock, s_ALUSrcA, s_RegDst 		: STD_LOGIC;
	SIGNAL s_Regwrite, s_Zero, s_MemWrite, s_MemRead, s_MemToReg	: STD_LOGIC;
	SIGNAL s_PCWrite, s_PCWriteCond, s_IRWrite, s_IorD 	       	: STD_LOGIC;	
	signal s_micropc 					      	: integer;

BEGIN
	

-- instantiate and connect the MIPS components   

  MC: my_clock -- instantiate the clock module 
  PORT MAP (sys_clock 	=> s_sys_clock,
            reset 	=> s_reset
            );

  IFE : Ifetch -- instantiate fetch module
PORT MAP (      PC_Plus_4    => s_ALU_Result,     -- connect all fetch ports to local signals
                Branch_PC    => s_ALUOut, 
                Jump_PC      => s_Jump_Address, -- connect Jump_PC to intermediate
                Jr_PC        => s_Jr_Address, -- connect Jr_PC to intermediate
        	PCWrite      => s_PCWrite,
        	PCWriteCond  => s_PCWriteCond,
        	IRWrite      => s_IRWrite,
        	PCSource     => s_PCSource,
        	IorD         => s_IorD,
                Zero         => s_Zero,
                MemRead      => s_MemRead,
                MemWrite     => s_MemWrite,    
        	clock        => s_sys_clock,
                reset        => s_reset,
                memory_data_in => s_read_data_2,  
        	memory_addr_in => s_ALUOut,
                PC_out 	     => s_PC_Out,  
                Memory_data_out 	=> s_memory_data_out,			
                Instruction  => s_instruction);  -- connect instruction to intermediate 

 ID : Idecode
  	PORT MAP (	
		read_data_1    	=> s_read_data_1,
		read_data_2    	=> s_read_data_2,
		Sign_extend 	=> s_Sign_extend,   
		Instruction     => s_instruction,
		read_data       => s_memory_data_out,
		ALU_result      => s_ALUOut,
		RegWrite        => s_RegWrite,
		MemtoReg        => s_MemToReg,
		RegDst 		=> s_RegDst,
		clock           => s_sys_clock,
		reset	        => s_reset);


   CTL:   control
	PORT MAP ( 
		Opcode  	=> s_instruction(31 downto 26), -- connect opcode to intermediate
                Function_opcode => s_instruction(5 downto 0), -- connect function opcode to intermediate
		clock           => s_sys_clock,
		reset	        => s_reset,
		PCWrite         => s_PCWrite,  
   		PCWriteCond     => s_PCWriteCond,
   		IorD            => s_IorD, 
   		MemRead  	=> s_MemRead,  
   		MemWrite 	=> s_MemWrite, 
   		IRWrite         => s_IRWrite,
   		MemtoReg        => s_MemToReg,
   		PCSource        => s_PCSource,
   		ALUOp           => s_ALUOp,
   		ALUSrcB 	=> s_ALUSrcB,
   		ALUSrcA         => s_ALUSrcA,
   		RegWrite 	=> s_RegWrite, 
   		RegDst          => s_RegDst,
		micropc 	=> s_micropc);

  EXE:  Execute
  	PORT MAP (	Read_data_1 	=> s_read_data_1,    
			Read_data_2 	=> s_read_data_2,
			Sign_extend 	=> s_sign_extend,
			Function_opcode => s_instruction(5 downto 0), -- connect function opcode to intermediate
			ALUOp 		=> s_ALUOp,
			ALUSrcB		=> s_ALUSrcB,
			ALUSrcA        	=> s_ALUSrcA,
			PC     	        => s_PC_Out,
			clock           => s_sys_clock,
			reset	        => s_reset,
			Zero 		=> s_Zero,
			ALU_Result      => s_ALU_Result,		 
			ALUOut 		=> s_ALUOut,
                        Instruction     => s_Instruction, 
                        Jr_Address      => s_Jr_Address, -- connect jr address to intermediate
                        Jump_Address    => s_Jump_Address); -- connect jump address to intermediate
END structure;