-- ECE 3056: Architecture, Concurrency and Energy in Computation
-- Sudhakar Yalamanchili
-- Multicycle MIPS Processor VHDL Behavioral Model
--
-- Execute module (implements the data ALU and Branch Address Adder)
--
-- School of Electrical & Computer Engineering
-- Georgia Institute of Technology
-- Atlanta, GA 30332

-- Sho Ko
-- 903197992
-- Add output signals Jump_Address and Jr_Address
 
LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_SIGNED.ALL;

ENTITY  Execute IS
	PORT(		Read_data_1 	: IN 	STD_LOGIC_VECTOR( 31 DOWNTO 0 );
			Read_data_2 	: IN 	STD_LOGIC_VECTOR( 31 DOWNTO 0 );
			Sign_extend 	: IN 	STD_LOGIC_VECTOR( 31 DOWNTO 0 );
			Function_opcode	: IN 	STD_LOGIC_VECTOR( 5 DOWNTO 0 );
			ALUOp 		: IN 	STD_LOGIC_VECTOR( 1 DOWNTO 0 );
			ALUSrcB		: IN 	STD_LOGIC_VECTOR( 1 DOWNTO 0 );
			ALUSrcA        	: IN 	STD_LOGIC;
			PC 	        : IN 	STD_LOGIC_VECTOR( 31 DOWNTO 0 );
                        Instruction     : IN    STD_LOGIC_VECTOR( 31 DOWNTO 0 ); -- input instruction
			clock, reset	: IN 	STD_LOGIC;
			
			Zero 	       	: OUT	STD_LOGIC;
			ALU_Result     	: OUT	STD_LOGIC_VECTOR( 31 DOWNTO 0 );
			ALUOut 		: OUT	STD_LOGIC_VECTOR( 31 DOWNTO 0 );
                        Jump_Address    : OUT   STD_LOGIC_VECTOR( 31 DOWNTO 0 ); -- output jump address
                        Jr_Address      : OUT   STD_LOGIC_VECTOR( 31 DOWNTO 0 )); -- output jr address
END Execute;

ARCHITECTURE behavior OF Execute IS
SIGNAL Ainput, Binput	: STD_LOGIC_VECTOR( 31 DOWNTO 0 ); 
signal ALU_Internal : std_logic_vector (31 downto 0);

SIGNAL ALU_ctl	: STD_LOGIC_VECTOR( 2 DOWNTO 0 );

BEGIN
         
        Jr_Address <= Read_data_1; -- jr address is rs
        Jump_Address <= "0000" & Instruction(25 downto 0) & "00"; -- jump address is lower 26 bits of instruction times 4

	Ainput <= Read_data_1 when ALUSrcA = '1' else
	          PC when ALUSrcA = '0' else
	          X"AAAAAAAA"; -- Indicates a problem in ALUSrcA value
						
	Binput <= Read_data_2 WHEN ( ALUSrcB = "00" ) else  -- for ALU operations
	         X"00000004" when ALUSrcB = "01" else       -- For PC+4
	         Sign_extend(31 downto 0) when ALUSrcB = "10" else   -- for immediate
	         (Sign_extend (29 downto 0) & "00") when ALUSrcB = "11" else --
                                                                             --branch
	         X"BBBBBBBB"; -- Indicates a problem in ALUSrcB value
	         
		-- Generate ALU control bits
		
	ALU_ctl( 0 ) <= ( Function_opcode( 0 ) OR Function_opcode( 3 ) ) AND ALUOp(1 );
	ALU_ctl( 1 ) <= ( NOT Function_opcode( 2 ) ) OR (NOT ALUOp( 1 ) );
	ALU_ctl( 2 ) <= ( Function_opcode( 1 ) AND ALUOp( 1 )) OR ALUOp( 0 );
						-- Generate Zero Flag
	Zero <= '1' WHEN ( ALU_internal = X"00000000"  )
		         ELSE '0';    	
		         			   
  ALU_result <= ALU_internal;					

PROCESS ( ALU_ctl, Ainput, Binput )
	BEGIN
					-- Select ALU operation
 	CASE ALU_ctl IS
						-- ALU performs ALUresult = A_input AND B_input
		WHEN "000" 	=>	ALU_internal 	<= Ainput AND Binput; 
						-- ALU performs ALUresult = A_input OR B_input
                WHEN "001" 	=>	ALU_internal 	<= Ainput OR Binput;
						-- ALU performs ALUresult = A_input + B_input
	 	WHEN "010" 	=>	ALU_internal 	<= Ainput + Binput;
						-- ALU performs ?
 	 	WHEN "011" 	=>	ALU_internal <= X"00000000";
						-- ALU performs ?
 	 	WHEN "100" 	=>	ALU_internal 	<= X"00000000";
						-- ALU performs ?
 	 	WHEN "101" 	=>	ALU_internal 	<=  X"00000000";
						-- ALU performs ALUresult = A_input -B_input
 	 	WHEN "110" 	=>	ALU_internal 	<= (Ainput - Binput);
						-- ALU performs SLT
  	 	WHEN "111" 	=>	ALU_internal 	<= (Ainput - Binput) ;
 	 	WHEN OTHERS	=>	ALU_internal 	<= X"FFFFFFFF" ; -- indicates a problem in ALU_ctl value
  	END CASE;
  END PROCESS;
  
  PROCESS
      BEGIN
          WAIT UNTIL (rising_edge(clock)); 
		  if (reset = '1') then 
		  ALUOut <= X"00000000"; 
		  else
          ALUOut <= ALU_internal(31 downto 0);
		  end if;
      end process; 
END behavior;