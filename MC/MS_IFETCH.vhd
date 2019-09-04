-- ECE 3056: Architecture, Concurrency and Energy in Computation
-- Sudhakar Yalamanchili
-- Multicycle MIPS Processor VHDL Behavioral Model
--
-- Ifetch module (provides the PC, instruction, and data memory) 
-- 
-- School of Electrical & Computer Engineering
-- Georgia Institute of Technology
-- Atlanta, GA 30332

-- Sho Ko
-- 903197992
-- Add input signals Jump_PC and Jr_PC to PCSource mux

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;

ENTITY Ifetch IS
	PORT(	-- Input signals
        	SIGNAL PC_Plus_4	       	: IN STD_LOGIC_VECTOR( 31 DOWNTO 0 );
        	SIGNAL Branch_PC            	: in STD_LOGIC_VECTOR( 31 DOWNTO 0 );
                SIGNAL Jump_PC                  : IN STD_LOGIC_VECTOR( 31 DOWNTO 0 ); -- input jump pc
                SIGNAL Jr_PC                    : IN STD_LOGIC_VECTOR( 31 DOWNTO 0 ); -- input jr pc
        	SIGNAL PCWrite, PCWriteCond, IRWrite : IN STD_LOGIC;
        	SIGNAL PCSource 	       	: IN STD_LOGIC_VECTOR(1 DOWNTO 0);
        	SIGNAL Zero, MemRead, MemWrite, IorD  : in STD_LOGIC;
        	SIGNAL clock, reset 		: IN STD_LOGIC;
        	SIGNAL memory_data_in   	: IN std_logic_vector(31 downto 0);
        	SIGNAL memory_addr_in   	: IN std_logic_vector(31 downto 0);
        	-- Output signals
        	SIGNAL PC_out 			: OUT	STD_LOGIC_VECTOR( 31 DOWNTO 0 );
        	SIGNAL Memory_data_out 		: OUT	STD_LOGIC_VECTOR( 31 DOWNTO 0 );
        	SIGNAL Instruction 		: OUT	STD_LOGIC_VECTOR( 31 DOWNTO 0 ));
END Ifetch;
														   
ARCHITECTURE behavior OF Ifetch IS
   TYPE INST_MEM IS ARRAY (0 to 7) of STD_LOGIC_VECTOR (31 DOWNTO 0);
   SIGNAL iram : INST_MEM := (
      X"00224820",     --add $t1, $1, $v0
      X"214a000c",     --addi $t2, $t2, 12
      X"ae0a0004",     --sw $t2, 4($s0)
      X"08000001",     --j 4
      X"00000000",
      X"00000000",
      X"00000000",
      X"00000000"              
   );
    
     TYPE DATA_RAM IS ARRAY (0 to 31) OF STD_LOGIC_VECTOR (31 DOWNTO 0);
   SIGNAL dram: DATA_RAM := (
      X"00000000",
      X"11111111",
      X"22222222",	-- Note data RAM is organized as by word not byte!
      X"33333333",
      X"44444444",
      X"55555555",
      X"66666666",
      X"77777777",
      X"0000000A",
      X"1111111A",
      X"2222222A",
      X"3333333A",
      X"4444444A",
      X"5555555A",
      X"6666666A",
      X"7777777A",
      X"0000000B",
      X"1111111B",
      X"2222222B",
      X"3333333B",
      X"4444444B",
      X"5555555B",
      X"6666666B",
      X"7777777B",
      X"000000BA",
      X"111111BA",
      X"222222BA",
      X"333333BA",
      X"444444BA",
      X"555555BA",
      X"666666BA",
      X"777777BA"
   );
	SIGNAL PC, Next_PC              : STD_LOGIC_VECTOR( 31 DOWNTO 0 );
	SIGNAL PC_Update 			: boolean; 
	SIGNAL Local_IR, Local_Data 	: std_logic_vector(31 downto 0);
BEGIN 						
					             
Local_IR <=  iram(CONV_INTEGER(PC(4 downto 2)))when IorD ='0' 
           else X"0000FFFF"; -- read instr pointed to by PC
Local_data <= dram(CONV_INTEGER(memory_addr_in(6 downto 2))) when IorD ='1'
           else X"00000000"; --read data from data addr		

-- compute value of next PC

Next_PC <=  PC_Plus_4    when PCSource = "00" else -- next instruction
            Branch_PC    when PCSource = "01" else -- branch
            Jump_PC      when PCSource = "10" else -- jump
            Jr_PC        when PCSource = "11" else -- jr
            X"CCCCCCCC"; -- indicates a problem with the value of PCSource
			   
-- check if the PC is to be updated on the next clock cycle
PC_Update <= ((PCWrite = '1') or ((PCWriteCond = '1') and (Zero ='1')));
	
	PROCESS
		BEGIN
			WAIT UNTIL (rising_edge(clock));
			IF (reset = '1') THEN
				PC<= X"00000000" ;
				pc_out <= x"00000000"; 
				Instruction <= x"00000000";
				Memory_data_out <= X"00000000";
			ELSE 
				if (PC_update) then 
					
						PC      <= next_PC; 	 
						PC_out  <= next_PC;
				end if;
			      if (IRWrite ='1') then Instruction <= Local_IR; end if;
			      IF (MemRead = '1')then Memory_data_out <= Local_Data; end if;
			      if (MemWrite = '1') then dram(CONV_INTEGER(memory_addr_in(6 downto 2))) 
			                              <= memory_data_in; end if;
			  	
			 end if; 
			  
	END PROCESS; 
			 
END behavior;