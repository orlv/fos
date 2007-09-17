#ifndef OPCODES_H_
#define OPCODES_H_

#define OPC_NOP                         0       /**/
#define OPC_ACONST_NULL                 1       /**/
#define OPC_ICONST_M1                   2
#define OPC_ICONST_0                    3       /**/
#define OPC_ICONST_1                    4       /**/
#define OPC_ICONST_2                    5       /**/
#define OPC_ICONST_3                    6       /**/
#define OPC_ICONST_4                    7       /**/
#define OPC_ICONST_5                    8       /**/
#define OPC_LCONST_0                    9       /**/
#define OPC_LCONST_1                    10      /**/
#define OPC_FCONST_0                    11
#define OPC_FCONST_1                    12
#define OPC_FCONST_2                    13
#define OPC_DCONST_0                    14
#define OPC_DCONST_1                    15
#define OPC_BIPUSH                      16      /**/
#define OPC_SIPUSH                      17      /**/
#define OPC_LDC                         18      /**/
#define OPC_LDC_W                       19      /**/
#define OPC_LDC2_W                      20      /**/
#define OPC_ILOAD                       21      /**/
#define OPC_LLOAD                       22      /**/
#define OPC_FLOAD                       23      /**/
#define OPC_DLOAD                       24
#define OPC_ALOAD                       25      /**/
#define OPC_ILOAD_0                     26      /**/
#define OPC_ILOAD_1                     27      /**/
#define OPC_ILOAD_2                     28      /**/
#define OPC_ILOAD_3                     29      /**/
#define OPC_LLOAD_0                     30      /**/
#define OPC_LLOAD_1                     31      /**/
#define OPC_LLOAD_2                     32      /**/
#define OPC_LLOAD_3                     33      /**/
#define OPC_FLOAD_0                     34
#define OPC_FLOAD_1                     35
#define OPC_FLOAD_2                     36
#define OPC_FLOAD_3                     37
#define OPC_DLOAD_0                     38
#define OPC_DLOAD_1                     39
#define OPC_DLOAD_2                     40
#define OPC_DLOAD_3                     41
#define OPC_ALOAD_0                     42      /**/
#define OPC_ALOAD_1                     43      /**/
#define OPC_ALOAD_2                     44      /**/
#define OPC_ALOAD_3                     45      /**/
#define OPC_IALOAD                      46      /**/
#define OPC_LALOAD                      47
#define OPC_FALOAD                      48
#define OPC_DALOAD                      49
#define OPC_AALOAD                      50      /**/
#define OPC_BALOAD                      51      /**/
#define OPC_CALOAD                      52      /**/
#define OPC_SALOAD                      53		/**/
#define OPC_ISTORE                      54      /**/
#define OPC_LSTORE                      55      /**/
#define OPC_FSTORE                      56
#define OPC_DSTORE                      57
#define OPC_ASTORE                      58      /**/
#define OPC_ISTORE_0                    59      /**/
#define OPC_ISTORE_1                    60      /**/
#define OPC_ISTORE_2                    61      /**/
#define OPC_ISTORE_3                    62      /**/
#define OPC_LSTORE_0                    63      /**/
#define OPC_LSTORE_1                    64      /**/
#define OPC_LSTORE_2                    65      /**/
#define OPC_LSTORE_3                    66      /**/
#define OPC_FSTORE_0                    67
#define OPC_FSTORE_1                    68
#define OPC_FSTORE_2                    69
#define OPC_FSTORE_3                    70
#define OPC_DSTORE_0                    71
#define OPC_DSTORE_1                    72
#define OPC_DSTORE_2                    73
#define OPC_DSTORE_3                    74
#define OPC_ASTORE_0                    75      /**/
#define OPC_ASTORE_1                    76      /**/
#define OPC_ASTORE_2                    77      /**/
#define OPC_ASTORE_3                    78      /**/
#define OPC_IASTORE                     79      /**/
#define OPC_LASTORE                     80
#define OPC_FASTORE                     81
#define OPC_DASTORE                     82
#define OPC_AASTORE                     83      /**/
#define OPC_BASTORE                     84      /**/
#define OPC_CASTORE                     85      /**/
#define OPC_SASTORE                     86
#define OPC_POP                         87      /**/
#define OPC_POP2                        88      /**/
#define OPC_DUP                         89      /**/
#define OPC_DUP_X1                      90
#define OPC_DUP_X2                      91
#define OPC_DUP2                        92
#define OPC_DUP2_X1                     93
#define OPC_DUP2_X2                     94
#define OPC_SWAP                        95      /**/
#define OPC_IADD                        96      /**/
#define OPC_LADD                        97      /**/
#define OPC_FADD                        98
#define OPC_DADD                        99
#define OPC_ISUB                        100     /**/
#define OPC_LSUB                        101     /**/
#define OPC_FSUB                        102
#define OPC_DSUB                        103
#define OPC_IMUL                        104     /**/
#define OPC_LMUL                        105     /**/
#define OPC_FMUL                        106
#define OPC_DMUL                        107
#define OPC_IDIV                        108     /**/
#define OPC_LDIV                        109
#define OPC_FDIV                        110
#define OPC_DDIV                        111
#define OPC_IREM                        112     /**/
#define OPC_LREM                        113
#define OPC_FREM                        114
#define OPC_DREM                        115
#define OPC_INEG                        116     /**/
#define OPC_LNEG                        117     /**/
#define OPC_FNEG                        118
#define OPC_DNEG                        119
#define OPC_ISHL                        120
#define OPC_LSHL                        121
#define OPC_ISHR                        122
#define OPC_LSHR                        123
#define OPC_IUSHR                       124
#define OPC_LUSHR                       125
#define OPC_IAND                        126
#define OPC_LAND                        127
#define OPC_IOR                         128
#define OPC_LOR                         129
#define OPC_IXOR                        130
#define OPC_LXOR                        131
#define OPC_IINC                        132     /**/
#define OPC_I2L                         133     /**/
#define OPC_I2F                         134
#define OPC_I2D                         135
#define OPC_L2I                         136     /**/
#define OPC_L2F                         137
#define OPC_L2D                         138
#define OPC_F2I                         139
#define OPC_F2L                         140
#define OPC_F2D                         141
#define OPC_D2I                         142
#define OPC_D2L                         143
#define OPC_D2F                         144
#define OPC_I2B                         145     /**/
#define OPC_I2C                         146     /**/
#define OPC_I2S                         147     /**/
#define OPC_LCMP                        148     /**/
#define OPC_FCMPL                       149
#define OPC_FCMPG                       150
#define OPC_DCMPL                       151
#define OPC_DCMPG                       152
#define OPC_IFEQ                        153     /**/
#define OPC_IFNE                        154     /**/
#define OPC_IFLT                        155     /**/
#define OPC_IFGE                        156     /**/
#define OPC_IFGT                        157     /**/
#define OPC_IFLE                        158     /**/
#define OPC_IF_ICMPEQ                   159     /**/
#define OPC_IF_ICMPNE                   160     /**/
#define OPC_IF_ICMPLT                   161     /**/
#define OPC_IF_ICMPGE                   162     /**/
#define OPC_IF_ICMPGT                   163     /**/
#define OPC_IF_ICMPLE                   164     /**/
#define OPC_IF_ACMPEQ                   165     /**/
#define OPC_IF_ACMPNE                   166     /**/
#define OPC_GOTO                        167     /**/
#define OPC_JSR                         168
#define OPC_RET                         169
#define OPC_TABLESWITCH                 170
#define OPC_LOOKUPSWITCH                171     /**/
#define OPC_IRETURN                     172     /**/
#define OPC_LRETURN                     173
#define OPC_FRETURN                     174
#define OPC_DRETURN                     175
#define OPC_ARETURN                     176     /**/
#define OPC_RETURN                      177     /**/
#define OPC_GETSTATIC                   178     /**/
#define OPC_PUTSTATIC                   179     /**/
#define OPC_GETFIELD                    180     /**/
#define OPC_PUTFIELD                    181     /**/
#define OPC_INVOKEVIRTUAL               182     /**/
#define OPC_INVOKESPECIAL               183     /**/
#define OPC_INVOKESTATIC                184     /**/
#define OPC_INVOKEINTERFACE             185     /**/
#define OPC_NEW                         187     /**/
#define OPC_NEWARRAY                    188     /**/
#define OPC_ANEWARRAY                   189     /**/
#define OPC_ARRAYLENGTH                 190     /**/
#define OPC_ATHROW                      191     /**/
#define OPC_CHECKCAST                   192     /**/
#define OPC_INSTANCEOF                  193     /**/
#define OPC_MONITORENTER                194     /**/
#define OPC_MONITOREXIT                 195     /**/
#define OPC_WIDE                        196
#define OPC_MULTIANEWARRAY              197
#define OPC_IFNULL                      198     /**/
#define OPC_IFNONNULL                   199     /**/
#define OPC_GOTO_W                      200
#define OPC_JSR_W                       201


#endif /*OPCODES_H_*/
