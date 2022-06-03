# Context switch
#
#   void swtch(struct context *old, struct context *new);
# # Save current registers in old. Load from new.	


.globl swtch
swtch:
        st.d $ra, $a0, 0
        st.d $sp, $a0, 8
        st.d $s0, $a0, 16
        st.d $s1, $a0, 24
        st.d $s2, $a0, 32
        st.d $s3, $a0, 40
        st.d $s4, $a0, 48
        st.d $s5, $a0, 56
        st.d $s6, $a0, 64
        st.d $s7, $a0, 72
        st.d $s8, $a0, 80
        st.d $fp, $a0, 88

        ld.d $ra, $a1, 0
        ld.d $sp, $a1, 8
        ld.d $s0, $a1, 16
        ld.d $s1, $a1, 24
        ld.d $s2, $a1, 32
        ld.d $s3, $a1, 40
        ld.d $s4, $a1, 48
        ld.d $s5, $a1, 56
        ld.d $s6, $a1, 64
        ld.d $s7, $a1, 72
        ld.d $s8, $a1, 80
        ld.d $fp, $a1, 88
        
        jirl $zero, $ra, 0

	
