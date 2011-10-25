/*
 * Interrupt service routines.
 */

  .file "isr.S"
  .text
  .align 4

.globl Interrupt0
Interrupt0:
  pushl $0
  jmp KernelEnter

.globl Interrupt1
Interrupt1:
  pushl $1
  jmp KernelEnter

.globl Interrupt2
Interrupt2:
  pushl $2
  jmp KernelEnter

.globl Interrupt3
Interrupt3:
  pushl $3
  jmp KernelEnter

.globl Interrupt4
Interrupt4:
  pushl $4
  jmp KernelEnter

.globl Interrupt5
Interrupt5:
  pushl $5
  jmp KernelEnter

.globl Interrupt6
Interrupt6:
  pushl $6
  jmp KernelEnter

.globl Interrupt7
Interrupt7:
  pushl $7
  jmp KernelEnter

.globl Interrupt8
Interrupt8:
  pushl $8
  jmp KernelEnter

.globl Interrupt9
Interrupt9:
  pushl $9
  jmp KernelEnter

.globl Interrupt10
Interrupt10:
  pushl $10
  jmp KernelEnter

.globl Interrupt11
Interrupt11:
  pushl $11
  jmp KernelEnter

.globl Interrupt12
Interrupt12:
  pushl $12
  jmp KernelEnter

.globl Interrupt13
Interrupt13:
  pushl $13
  jmp KernelEnter

.globl Interrupt14
Interrupt14:
  pushl $14
  jmp KernelEnter

.globl Interrupt15
Interrupt15:
  pushl $15
  jmp KernelEnter

.globl Interrupt16
Interrupt16:
  pushl $16
  jmp KernelEnter

.globl Interrupt17
Interrupt17:
  pushl $17
  jmp KernelEnter

.globl Interrupt18
Interrupt18:
  pushl $18
  jmp KernelEnter

.globl Interrupt19
Interrupt19:
  pushl $19
  jmp KernelEnter

.globl Interrupt20
Interrupt20:
  pushl $20
  jmp KernelEnter

.globl Interrupt21
Interrupt21:
  pushl $21
  jmp KernelEnter

.globl Interrupt22
Interrupt22:
  pushl $22
  jmp KernelEnter

.globl Interrupt23
Interrupt23:
  pushl $23
  jmp KernelEnter

.globl Interrupt24
Interrupt24:
  pushl $24
  jmp KernelEnter

.globl Interrupt25
Interrupt25:
  pushl $25
  jmp KernelEnter

.globl Interrupt26
Interrupt26:
  pushl $26
  jmp KernelEnter

.globl Interrupt27
Interrupt27:
  pushl $27
  jmp KernelEnter

.globl Interrupt28
Interrupt28:
  pushl $28
  jmp KernelEnter

.globl Interrupt29
Interrupt29:
  pushl $29
  jmp KernelEnter

.globl Interrupt30
Interrupt30:
  pushl $30
  jmp KernelEnter

.globl Interrupt31
Interrupt31:
  pushl $31
  jmp KernelEnter

.globl Interrupt32
Interrupt32:
  pushl $32
  jmp KernelEnter

.globl Interrupt33
Interrupt33:
  pushl $33
  jmp KernelEnter

.globl Interrupt34
Interrupt34:
  pushl $34
  jmp KernelEnter

.globl Interrupt35
Interrupt35:
  pushl $35
  jmp KernelEnter

.globl Interrupt36
Interrupt36:
  pushl $36
  jmp KernelEnter

.globl Interrupt37
Interrupt37:
  pushl $37
  jmp KernelEnter

.globl Interrupt38
Interrupt38:
  pushl $38
  jmp KernelEnter

.globl Interrupt39
Interrupt39:
  pushl $39
  jmp KernelEnter

.globl Interrupt40
Interrupt40:
  pushl $40
  jmp KernelEnter

.globl Interrupt41
Interrupt41:
  pushl $41
  jmp KernelEnter

.globl Interrupt42
Interrupt42:
  pushl $42
  jmp KernelEnter

.globl Interrupt43
Interrupt43:
  pushl $43
  jmp KernelEnter

.globl Interrupt44
Interrupt44:
  pushl $44
  jmp KernelEnter

.globl Interrupt45
Interrupt45:
  pushl $45
  jmp KernelEnter

.globl Interrupt46
Interrupt46:
  pushl $46
  jmp KernelEnter

.globl Interrupt47
Interrupt47:
  pushl $47
  jmp KernelEnter

.globl Interrupt48
Interrupt48:
  pushl $48
  jmp KernelEnter

.globl Interrupt49
Interrupt49:
  pushl $49
  jmp KernelEnter

.globl Interrupt50
Interrupt50:
  pushl $50
  jmp KernelEnter

.globl Interrupt51
Interrupt51:
  pushl $51
  jmp KernelEnter

.globl Interrupt52
Interrupt52:
  pushl $52
  jmp KernelEnter

.globl Interrupt53
Interrupt53:
  pushl $53
  jmp KernelEnter

.globl Interrupt54
Interrupt54:
  pushl $54
  jmp KernelEnter

.globl Interrupt55
Interrupt55:
  pushl $55
  jmp KernelEnter

.globl Interrupt56
Interrupt56:
  pushl $56
  jmp KernelEnter

.globl Interrupt57
Interrupt57:
  pushl $57
  jmp KernelEnter

.globl Interrupt58
Interrupt58:
  pushl $58
  jmp KernelEnter

.globl Interrupt59
Interrupt59:
  pushl $59
  jmp KernelEnter

.globl Interrupt60
Interrupt60:
  pushl $60
  jmp KernelEnter

.globl Interrupt61
Interrupt61:
  pushl $61
  jmp KernelEnter

.globl Interrupt62
Interrupt62:
  pushl $62
  jmp KernelEnter

.globl Interrupt63
Interrupt63:
  pushl $63
  jmp KernelEnter

.globl Interrupt64
Interrupt64:
  pushl $64
  jmp KernelEnter

.globl Interrupt65
Interrupt65:
  pushl $65
  jmp KernelEnter

.globl Interrupt66
Interrupt66:
  pushl $66
  jmp KernelEnter

.globl Interrupt67
Interrupt67:
  pushl $67
  jmp KernelEnter

.globl Interrupt68
Interrupt68:
  pushl $68
  jmp KernelEnter

.globl Interrupt69
Interrupt69:
  pushl $69
  jmp KernelEnter

.globl Interrupt70
Interrupt70:
  pushl $70
  jmp KernelEnter

.globl Interrupt71
Interrupt71:
  pushl $71
  jmp KernelEnter

.globl Interrupt72
Interrupt72:
  pushl $72
  jmp KernelEnter

.globl Interrupt73
Interrupt73:
  pushl $73
  jmp KernelEnter

.globl Interrupt74
Interrupt74:
  pushl $74
  jmp KernelEnter

.globl Interrupt75
Interrupt75:
  pushl $75
  jmp KernelEnter

.globl Interrupt76
Interrupt76:
  pushl $76
  jmp KernelEnter

.globl Interrupt77
Interrupt77:
  pushl $77
  jmp KernelEnter

.globl Interrupt78
Interrupt78:
  pushl $78
  jmp KernelEnter

.globl Interrupt79
Interrupt79:
  pushl $79
  jmp KernelEnter

.globl Interrupt80
Interrupt80:
  pushl $80
  jmp KernelEnter

.globl Interrupt81
Interrupt81:
  pushl $81
  jmp KernelEnter

.globl Interrupt82
Interrupt82:
  pushl $82
  jmp KernelEnter

.globl Interrupt83
Interrupt83:
  pushl $83
  jmp KernelEnter

.globl Interrupt84
Interrupt84:
  pushl $84
  jmp KernelEnter

.globl Interrupt85
Interrupt85:
  pushl $85
  jmp KernelEnter

.globl Interrupt86
Interrupt86:
  pushl $86
  jmp KernelEnter

.globl Interrupt87
Interrupt87:
  pushl $87
  jmp KernelEnter

.globl Interrupt88
Interrupt88:
  pushl $88
  jmp KernelEnter

.globl Interrupt89
Interrupt89:
  pushl $89
  jmp KernelEnter

.globl Interrupt90
Interrupt90:
  pushl $90
  jmp KernelEnter

.globl Interrupt91
Interrupt91:
  pushl $91
  jmp KernelEnter

.globl Interrupt92
Interrupt92:
  pushl $92
  jmp KernelEnter

.globl Interrupt93
Interrupt93:
  pushl $93
  jmp KernelEnter

.globl Interrupt94
Interrupt94:
  pushl $94
  jmp KernelEnter

.globl Interrupt95
Interrupt95:
  pushl $95
  jmp KernelEnter

.globl Interrupt96
Interrupt96:
  pushl $96
  jmp KernelEnter

.globl Interrupt97
Interrupt97:
  pushl $97
  jmp KernelEnter

.globl Interrupt98
Interrupt98:
  pushl $98
  jmp KernelEnter

.globl Interrupt99
Interrupt99:
  pushl $99
  jmp KernelEnter

.globl Interrupt100
Interrupt100:
  pushl $100
  jmp KernelEnter

.globl Interrupt101
Interrupt101:
  pushl $101
  jmp KernelEnter

.globl Interrupt102
Interrupt102:
  pushl $102
  jmp KernelEnter

.globl Interrupt103
Interrupt103:
  pushl $103
  jmp KernelEnter

.globl Interrupt104
Interrupt104:
  pushl $104
  jmp KernelEnter

.globl Interrupt105
Interrupt105:
  pushl $105
  jmp KernelEnter

.globl Interrupt106
Interrupt106:
  pushl $106
  jmp KernelEnter

.globl Interrupt107
Interrupt107:
  pushl $107
  jmp KernelEnter

.globl Interrupt108
Interrupt108:
  pushl $108
  jmp KernelEnter

.globl Interrupt109
Interrupt109:
  pushl $109
  jmp KernelEnter

.globl Interrupt110
Interrupt110:
  pushl $110
  jmp KernelEnter

.globl Interrupt111
Interrupt111:
  pushl $111
  jmp KernelEnter

.globl Interrupt112
Interrupt112:
  pushl $112
  jmp KernelEnter

.globl Interrupt113
Interrupt113:
  pushl $113
  jmp KernelEnter

.globl Interrupt114
Interrupt114:
  pushl $114
  jmp KernelEnter

.globl Interrupt115
Interrupt115:
  pushl $115
  jmp KernelEnter

.globl Interrupt116
Interrupt116:
  pushl $116
  jmp KernelEnter

.globl Interrupt117
Interrupt117:
  pushl $117
  jmp KernelEnter

.globl Interrupt118
Interrupt118:
  pushl $118
  jmp KernelEnter

.globl Interrupt119
Interrupt119:
  pushl $119
  jmp KernelEnter

.globl Interrupt120
Interrupt120:
  pushl $120
  jmp KernelEnter

.globl Interrupt121
Interrupt121:
  pushl $121
  jmp KernelEnter

.globl Interrupt122
Interrupt122:
  pushl $122
  jmp KernelEnter

.globl Interrupt123
Interrupt123:
  pushl $123
  jmp KernelEnter

.globl Interrupt124
Interrupt124:
  pushl $124
  jmp KernelEnter

.globl Interrupt125
Interrupt125:
  pushl $125
  jmp KernelEnter

.globl Interrupt126
Interrupt126:
  pushl $126
  jmp KernelEnter

.globl Interrupt127
Interrupt127:
  pushl $127
  jmp KernelEnter

.globl Interrupt128
Interrupt128:
  pushl $128
  jmp KernelEnter

.globl Interrupt129
Interrupt129:
  pushl $129
  jmp KernelEnter

.globl Interrupt130
Interrupt130:
  pushl $130
  jmp KernelEnter

.globl Interrupt131
Interrupt131:
  pushl $131
  jmp KernelEnter

.globl Interrupt132
Interrupt132:
  pushl $132
  jmp KernelEnter

.globl Interrupt133
Interrupt133:
  pushl $133
  jmp KernelEnter

.globl Interrupt134
Interrupt134:
  pushl $134
  jmp KernelEnter

.globl Interrupt135
Interrupt135:
  pushl $135
  jmp KernelEnter

.globl Interrupt136
Interrupt136:
  pushl $136
  jmp KernelEnter

.globl Interrupt137
Interrupt137:
  pushl $137
  jmp KernelEnter

.globl Interrupt138
Interrupt138:
  pushl $138
  jmp KernelEnter

.globl Interrupt139
Interrupt139:
  pushl $139
  jmp KernelEnter

.globl Interrupt140
Interrupt140:
  pushl $140
  jmp KernelEnter

.globl Interrupt141
Interrupt141:
  pushl $141
  jmp KernelEnter

.globl Interrupt142
Interrupt142:
  pushl $142
  jmp KernelEnter

.globl Interrupt143
Interrupt143:
  pushl $143
  jmp KernelEnter

.globl Interrupt144
Interrupt144:
  pushl $144
  jmp KernelEnter

.globl Interrupt145
Interrupt145:
  pushl $145
  jmp KernelEnter

.globl Interrupt146
Interrupt146:
  pushl $146
  jmp KernelEnter

.globl Interrupt147
Interrupt147:
  pushl $147
  jmp KernelEnter

.globl Interrupt148
Interrupt148:
  pushl $148
  jmp KernelEnter

.globl Interrupt149
Interrupt149:
  pushl $149
  jmp KernelEnter

.globl Interrupt150
Interrupt150:
  pushl $150
  jmp KernelEnter

.globl Interrupt151
Interrupt151:
  pushl $151
  jmp KernelEnter

.globl Interrupt152
Interrupt152:
  pushl $152
  jmp KernelEnter

.globl Interrupt153
Interrupt153:
  pushl $153
  jmp KernelEnter

.globl Interrupt154
Interrupt154:
  pushl $154
  jmp KernelEnter

.globl Interrupt155
Interrupt155:
  pushl $155
  jmp KernelEnter

.globl Interrupt156
Interrupt156:
  pushl $156
  jmp KernelEnter

.globl Interrupt157
Interrupt157:
  pushl $157
  jmp KernelEnter

.globl Interrupt158
Interrupt158:
  pushl $158
  jmp KernelEnter

.globl Interrupt159
Interrupt159:
  pushl $159
  jmp KernelEnter

.globl Interrupt160
Interrupt160:
  pushl $160
  jmp KernelEnter

.globl Interrupt161
Interrupt161:
  pushl $161
  jmp KernelEnter

.globl Interrupt162
Interrupt162:
  pushl $162
  jmp KernelEnter

.globl Interrupt163
Interrupt163:
  pushl $163
  jmp KernelEnter

.globl Interrupt164
Interrupt164:
  pushl $164
  jmp KernelEnter

.globl Interrupt165
Interrupt165:
  pushl $165
  jmp KernelEnter

.globl Interrupt166
Interrupt166:
  pushl $166
  jmp KernelEnter

.globl Interrupt167
Interrupt167:
  pushl $167
  jmp KernelEnter

.globl Interrupt168
Interrupt168:
  pushl $168
  jmp KernelEnter

.globl Interrupt169
Interrupt169:
  pushl $169
  jmp KernelEnter

.globl Interrupt170
Interrupt170:
  pushl $170
  jmp KernelEnter

.globl Interrupt171
Interrupt171:
  pushl $171
  jmp KernelEnter

.globl Interrupt172
Interrupt172:
  pushl $172
  jmp KernelEnter

.globl Interrupt173
Interrupt173:
  pushl $173
  jmp KernelEnter

.globl Interrupt174
Interrupt174:
  pushl $174
  jmp KernelEnter

.globl Interrupt175
Interrupt175:
  pushl $175
  jmp KernelEnter

.globl Interrupt176
Interrupt176:
  pushl $176
  jmp KernelEnter

.globl Interrupt177
Interrupt177:
  pushl $177
  jmp KernelEnter

.globl Interrupt178
Interrupt178:
  pushl $178
  jmp KernelEnter

.globl Interrupt179
Interrupt179:
  pushl $179
  jmp KernelEnter

.globl Interrupt180
Interrupt180:
  pushl $180
  jmp KernelEnter

.globl Interrupt181
Interrupt181:
  pushl $181
  jmp KernelEnter

.globl Interrupt182
Interrupt182:
  pushl $182
  jmp KernelEnter

.globl Interrupt183
Interrupt183:
  pushl $183
  jmp KernelEnter

.globl Interrupt184
Interrupt184:
  pushl $184
  jmp KernelEnter

.globl Interrupt185
Interrupt185:
  pushl $185
  jmp KernelEnter

.globl Interrupt186
Interrupt186:
  pushl $186
  jmp KernelEnter

.globl Interrupt187
Interrupt187:
  pushl $187
  jmp KernelEnter

.globl Interrupt188
Interrupt188:
  pushl $188
  jmp KernelEnter

.globl Interrupt189
Interrupt189:
  pushl $189
  jmp KernelEnter

.globl Interrupt190
Interrupt190:
  pushl $190
  jmp KernelEnter

.globl Interrupt191
Interrupt191:
  pushl $191
  jmp KernelEnter

.globl Interrupt192
Interrupt192:
  pushl $192
  jmp KernelEnter

.globl Interrupt193
Interrupt193:
  pushl $193
  jmp KernelEnter

.globl Interrupt194
Interrupt194:
  pushl $194
  jmp KernelEnter

.globl Interrupt195
Interrupt195:
  pushl $195
  jmp KernelEnter

.globl Interrupt196
Interrupt196:
  pushl $196
  jmp KernelEnter

.globl Interrupt197
Interrupt197:
  pushl $197
  jmp KernelEnter

.globl Interrupt198
Interrupt198:
  pushl $198
  jmp KernelEnter

.globl Interrupt199
Interrupt199:
  pushl $199
  jmp KernelEnter

.globl Interrupt200
Interrupt200:
  pushl $200
  jmp KernelEnter

.globl Interrupt201
Interrupt201:
  pushl $201
  jmp KernelEnter

.globl Interrupt202
Interrupt202:
  pushl $202
  jmp KernelEnter

.globl Interrupt203
Interrupt203:
  pushl $203
  jmp KernelEnter

.globl Interrupt204
Interrupt204:
  pushl $204
  jmp KernelEnter

.globl Interrupt205
Interrupt205:
  pushl $205
  jmp KernelEnter

.globl Interrupt206
Interrupt206:
  pushl $206
  jmp KernelEnter

.globl Interrupt207
Interrupt207:
  pushl $207
  jmp KernelEnter

.globl Interrupt208
Interrupt208:
  pushl $208
  jmp KernelEnter

.globl Interrupt209
Interrupt209:
  pushl $209
  jmp KernelEnter

.globl Interrupt210
Interrupt210:
  pushl $210
  jmp KernelEnter

.globl Interrupt211
Interrupt211:
  pushl $211
  jmp KernelEnter

.globl Interrupt212
Interrupt212:
  pushl $212
  jmp KernelEnter

.globl Interrupt213
Interrupt213:
  pushl $213
  jmp KernelEnter

.globl Interrupt214
Interrupt214:
  pushl $214
  jmp KernelEnter

.globl Interrupt215
Interrupt215:
  pushl $215
  jmp KernelEnter

.globl Interrupt216
Interrupt216:
  pushl $216
  jmp KernelEnter

.globl Interrupt217
Interrupt217:
  pushl $217
  jmp KernelEnter

.globl Interrupt218
Interrupt218:
  pushl $218
  jmp KernelEnter

.globl Interrupt219
Interrupt219:
  pushl $219
  jmp KernelEnter

.globl Interrupt220
Interrupt220:
  pushl $220
  jmp KernelEnter

.globl Interrupt221
Interrupt221:
  pushl $221
  jmp KernelEnter

.globl Interrupt222
Interrupt222:
  pushl $222
  jmp KernelEnter

.globl Interrupt223
Interrupt223:
  pushl $223
  jmp KernelEnter

.globl Interrupt224
Interrupt224:
  pushl $224
  jmp KernelEnter

.globl Interrupt225
Interrupt225:
  pushl $225
  jmp KernelEnter

.globl Interrupt226
Interrupt226:
  pushl $226
  jmp KernelEnter

.globl Interrupt227
Interrupt227:
  pushl $227
  jmp KernelEnter

.globl Interrupt228
Interrupt228:
  pushl $228
  jmp KernelEnter

.globl Interrupt229
Interrupt229:
  pushl $229
  jmp KernelEnter

.globl Interrupt230
Interrupt230:
  pushl $230
  jmp KernelEnter

.globl Interrupt231
Interrupt231:
  pushl $231
  jmp KernelEnter

.globl Interrupt232
Interrupt232:
  pushl $232
  jmp KernelEnter

.globl Interrupt233
Interrupt233:
  pushl $233
  jmp KernelEnter

.globl Interrupt234
Interrupt234:
  pushl $234
  jmp KernelEnter

.globl Interrupt235
Interrupt235:
  pushl $235
  jmp KernelEnter

.globl Interrupt236
Interrupt236:
  pushl $236
  jmp KernelEnter

.globl Interrupt237
Interrupt237:
  pushl $237
  jmp KernelEnter

.globl Interrupt238
Interrupt238:
  pushl $238
  jmp KernelEnter

.globl Interrupt239
Interrupt239:
  pushl $239
  jmp KernelEnter

.globl Interrupt240
Interrupt240:
  pushl $240
  jmp KernelEnter

.globl Interrupt241
Interrupt241:
  pushl $241
  jmp KernelEnter

.globl Interrupt242
Interrupt242:
  pushl $242
  jmp KernelEnter

.globl Interrupt243
Interrupt243:
  pushl $243
  jmp KernelEnter

.globl Interrupt244
Interrupt244:
  pushl $244
  jmp KernelEnter

.globl Interrupt245
Interrupt245:
  pushl $245
  jmp KernelEnter

.globl Interrupt246
Interrupt246:
  pushl $246
  jmp KernelEnter

.globl Interrupt247
Interrupt247:
  pushl $247
  jmp KernelEnter

.globl Interrupt248
Interrupt248:
  pushl $248
  jmp KernelEnter

.globl Interrupt249
Interrupt249:
  pushl $249
  jmp KernelEnter

.globl Interrupt250
Interrupt250:
  pushl $250
  jmp KernelEnter

.globl Interrupt251
Interrupt251:
  pushl $251
  jmp KernelEnter

.globl Interrupt252
Interrupt252:
  pushl $252
  jmp KernelEnter

.globl Interrupt253
Interrupt253:
  pushl $253
  jmp KernelEnter

.globl Interrupt254
Interrupt254:
  pushl $254
  jmp KernelEnter

.globl Interrupt255
Interrupt255:
  pushl $255
  jmp KernelEnter

