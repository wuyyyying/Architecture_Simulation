-- ECE 3056: Architecture, Concurrency and Energy in Computation
-- Sudhakar Yalamanchili
-- Pipelined MIPS Processor VHDL Behavioral Mode--
--
--
-- execution unit. only a subset of instructions are supported in this
-- model, specifically add, sub, lw, sw, beq, and, or

-- Sho Ko
-- 903197992
-- Add data hazard detection and forwarding logic, change ALU input muxes.

Library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_signed.all;

entity execute is
port(
--
-- inputs
-- 
     PC4 : in std_logic_vector(31 downto 0);
     register_rs, register_rt, mem_alu_result, wb_memory_data, wb_alu_result :in std_logic_vector (31 downto 0); -- add input signals for forwarding
     Sign_extend :in std_logic_vector(31 downto 0);
     ALUOp: in std_logic_vector(1 downto 0);
     ALUSrc, RegDst, wb_RegWrite, mem_RegWrite, clk, reset, wb_MemToReg : in std_logic; -- add input signals for hazard detection
     wreg_rd, wreg_rt, wreg_rs, wb_wreg_addr, mem_wreg_addr : in std_logic_vector(4 downto 0); -- add input signals for hazard detection

-- outputs
--
     alu_result, Branch_PC, writeB :out std_logic_vector(31 downto 0); -- add ourput writeB signal for sw
     wreg_address : out std_logic_vector(4 downto 0);
     zero: out std_logic);    
     end execute;

architecture behavioral of execute is 
SIGNAL Ainput, Binput, Xinput	: STD_LOGIC_VECTOR( 31 DOWNTO 0 ); 
SIGNAL ForwardA, ForwardB : STD_LOGIC_VECTOR( 1 DOWNTO 0 );
signal ALU_Internal : std_logic_vector (31 downto 0);
Signal Function_opcode : std_logic_vector (5 downto 0);

SIGNAL ALU_ctl	: STD_LOGIC_VECTOR( 2 DOWNTO 0 );

BEGIN
	-- first mux
	Ainput <= register_rs    when ForwardA = "00" else
		  wb_memory_data when ForwardA = "01" else
	    	  mem_alu_result when ForwardA = "10";
	

	-- second mux
	Xinput <= register_rt     when ( ForwardB ="00" ) else
	          wb_memory_data  when (ForwardB ="01" and wb_MemToReg ='1') else
	          wb_alu_result   when (ForwardB ="01" and wb_MemToReg = '0' ) else
	          mem_alu_result  when (ForwardB = "10") else
	          X"BBBBBBBB";

	-- ALUSrc mux
	Binput <= Sign_extend(31 downto 0) when (ALUSrc ='1') else 
                  Xinput;

	writeB <= Binput;

--END PROCESS;
	         
	 Branch_PC <= PC4 + (Sign_extend(29 downto 0) & "00");
	 
	 -- Get the function field. This will be the least significant
	 -- 6 bits of  the sign extended offset
	 
	 Function_opcode <= Sign_extend(5 downto 0);
	         
		-- Generate ALU control bits
		
	ALU_ctl( 0 ) <= ( Function_opcode( 0 ) OR Function_opcode( 3 ) ) AND ALUOp(1 );
	ALU_ctl( 1 ) <= ( NOT Function_opcode( 2 ) ) OR (NOT ALUOp( 1 ) );
	ALU_ctl( 2 ) <= ( Function_opcode( 1 ) AND ALUOp( 1 )) OR ALUOp( 0 );
		
		-- Generate Zero Flag
	Zero <= '1' WHEN ( ALU_internal = X"00000000")
		         ELSE '0';	
		         
-- implement the RegDst mux in this pipeline stage
--
wreg_address <= wreg_rd when RegDst = '1' else wreg_rt;
		         			   
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
 	 	WHEN OTHERS	=>	ALU_internal 	<= X"FFFFFFFF" ;
  	END CASE;
  END PROCESS;

-- implement hazard detection logic for rs
ForwardA <= "01" when ((wb_RegWrite = '1' and wb_wreg_addr /= "00000") and not(mem_RegWrite = '1' and mem_wreg_addr /= "00000" and mem_wreg_addr = wreg_rs) and (wb_wreg_addr = wreg_rs)) else
	    "10" when ((mem_RegWrite = '1' and mem_wreg_addr /= "00000") and (mem_wreg_addr = wreg_rs)) else
	    "00";

-- implement hazard detection logic for rt
ForwardB <= "01" when (wb_RegWrite = '1' and wb_wreg_addr /= "00000") and not((mem_RegWrite = '1') and (mem_wreg_addr /= "00000") and (mem_wreg_addr = wreg_rt)) and (wb_wreg_addr = wreg_rt) else
	    "10" when (mem_RegWrite = '1' and mem_wreg_addr /= "00000" and mem_wreg_addr = wreg_rt) else 
	    "00";
end behavioral;