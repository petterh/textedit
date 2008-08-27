//	#define WBF_CLASS			((BYTE) 0x0F)
//	#define WBF_ISWHITE			((BYTE) 0x10)
//	#define WBF_BREAKLINE		((BYTE) 0x20)
//	#define WBF_BREAKAFTER		((BYTE) 0x40)

int CharClasses[ 256 ] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-8
	WBF_ISWHITE | 3, // TAB = 16 + 3 = 1011 = 0x13 = WBF_ISWHITE | 3
	WBF_ISWHITE | 4, WBF_ISWHITE | 4, WBF_ISWHITE | 4, WBF_ISWHITE | 4, // 10-13
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 14-31
	WBF_ISWHITE | WBF_BREAKLINE | 2, // SPACE
	1, // 33: !
	1, // 34: " 
	1, // 35: #
	1, // 36: $
	1, // 37: %
	1, // 38: &
	1, // 39: '
	1, // 40: (
	1, // 41: )
	1, // 42: *
	1, // 43: +
	1, // 44: ,
	WBF_BREAKAFTER | 1, // 45: -
	1, // 46: .
	1, // 47: /
	0, // 48: 0
	0, // 49: 1
	0, // 50: 2
	0, // 51: 3
	0, // 52: 4
	0, // 53: 5
	0, // 54: 6
	0, // 55: 7
	0, // 56: 8
	0, // 57: 9
	1, // 58: :
	1, // 59: ;
	1, // 60: <
	1, // 61: =
	1, // 62: >
	1, // 63: ?
	1, // 64: @
	0, // 65: A
	0, // 66: B
	0, // 67: C
	0, // 68: D
	0, // 69: E
	0, // 70: F
	0, // 71: G
	0, // 72: H
	0, // 73: I
	0, // 74: J
	0, // 75: K
	0, // 76: L
	0, // 77: M
	0, // 78: N
	0, // 79: O
	0, // 80: P
	0, // 81: Q
	0, // 82: R
	0, // 83: S
	0, // 84: T
	0, // 85: U
	0, // 86: V
	0, // 87: W
	0, // 88: X
	0, // 89: Y
	0, // 90: Z
	1, // 91: [
	1, // 92: \ --watch out for ending a line with a backslash!
	1, // 93: ]
	1, // 94: ^
	0, // 95: _ // Changed from 1
	1, // 96: `
	0, // 97: a
	0, // 98: b
	0, // 99: c
	0, // 100: d
	0, // 101: e
	0, // 102: f
	0, // 103: g
	0, // 104: h
	0, // 105: i
	0, // 106: j
	0, // 107: k
	0, // 108: l
	0, // 109: m
	0, // 110: n
	0, // 111: o
	0, // 112: p
	0, // 113: q
	0, // 114: r
	0, // 115: s
	0, // 116: t
	0, // 117: u
	0, // 118: v
	0, // 119: w
	0, // 120: x
	0, // 121: y
	0, // 122: z
	1, // 123: {
	1, // 124: |
	1, // 125: }
	1, // 126: ~
	0, // 127: 

	// This section seems not to be used in CharMap
	0, // 128: ? // DEL
	0, // 129: Å
	1, // 130: ?
	0, // 131: ?
	1, // 132: ?
	1, // 133: ?
	1, // 134: ?
	1, // 135: ?
	0, // 136: ?
	1, // 137: ?
	0, // 138: ?
	1, // 139: ?
	0, // 140: ?
	0, // 141: ç
	0, // 142: ?
	0, // 143: è
	0, // 144: ê
	1, // 145: ?
	1, // 146: ?
	1, // 147: ?
	1, // 148: ?
	1, // 149: ?
	1, // 150: ?
	1, // 151: ?
	0, // 152: ?
	0, // 153: ?
	0, // 154: ?
	1, // 155: ?
	0, // 156: ?
	0, // 157: ù
	0, // 158: ?
	0, // 159: ?
	// End of CharMap unused section

	WBF_ISWHITE | 2, // 160: // non-breaking space (try Ctrl+Shift+Space)
	1, // 161: °
	1, // 162: ¢
	1, // 163: £
	1, // 164: §
	1, // 165: •
	1, // 166: ¶
	1, // 167: ß
	1, // 168: ®
	1, // 169: ©
	1, // 170: ™
	1, // 171: ´
	1, // 172: ¨
	1, // 173: ≠
	1, // 174: Æ
	1, // 175: Ø
	1, // 176: ∞
	1, // 177: ±
	1, // 178: ≤
	1, // 179: ≥
	1, // 180: ¥
	1, // 181: µ
	1, // 182: ∂
	1, // 183: ∑
	1, // 184: ∏
	1, // 185: π
	1, // 186: ∫
	1, // 187: ª
	1, // 188: º
	1, // 189: Ω
	1, // 190: æ
	1, // 191: ø
	0, // 192: ¿
	0, // 193: ¡
	0, // 194: ¬
	0, // 195: √
	0, // 196: ƒ
	0, // 197: ≈
	0, // 198: ∆
	0, // 199: «
	0, // 200: »
	0, // 201: …
	0, // 202:  
	0, // 203: À
	0, // 204: Ã
	0, // 205: Õ
	0, // 206: Œ
	0, // 207: œ
	0, // 208: –
	0, // 209: —
	0, // 210: “
	0, // 211: ”
	0, // 212: ‘
	0, // 213: ’
	0, // 214: ÷
	1, // 215: ◊
	0, // 216: ÿ
	0, // 217: Ÿ
	0, // 218: ⁄
	0, // 219: €
	0, // 220: ‹
	0, // 221: ›
	0, // 222: ﬁ
	0, // 223: ﬂ
	0, // 224: ‡
	0, // 225: ·
	0, // 226: ‚
	0, // 227: „
	0, // 228: ‰
	0, // 229: Â
	0, // 230: Ê
	0, // 231: Á
	0, // 232: Ë
	0, // 233: È
	0, // 234: Í
	0, // 235: Î
	0, // 236: Ï
	0, // 237: Ì
	0, // 238: Ó
	0, // 239: Ô
	0, // 240: 
	0, // 241: Ò
	0, // 242: Ú
	0, // 243: Û
	0, // 244: Ù
	0, // 245: ı
	0, // 246: ˆ
	1, // 247: ˜
	0, // 248: ¯
	0, // 249: ˘
	0, // 250: ˙
	0, // 251: ˚
	0, // 252: ¸
	0, // 253: ˝
	0, // 254: ˛
	0, // 255: ˇ	
};

/*
  0: 0 
  1: 0 
  2: 0 
  3: 0 
  4: 0 
  5: 0 
  6: 0 
  7: 0 
  8: 0 
  9: 19 	
 10: 20 
 11: 20 
 12: 20 
 13: 20 
 14: 0 
 15: 0 
 16: 0 
 17: 0 
 18: 0 
 19: 0 
 20: 0 
 21: 0 
 22: 0 
 23: 0 
 24: 0 
 25: 0 
 26: 0 
 27: 0 
 28: 0 
 29: 0 
 30: 0 
 31: 0 
 32: 50  
 33: 1 !
 34: 1 "
 35: 1 #
 36: 1 $
 37: 1 %
 38: 1 &
 39: 1 '
 40: 1 (
 41: 1 )
FindWordBreak: 1
 42: 1 *
FindWordBreak: 1
 43: 1 +
FindWordBreak: 1
 44: 1 ,
FindWordBreak: 65
 45: 65 -
FindWordBreak: 1
 46: 1 .
FindWordBreak: 1
 47: 1 /
FindWordBreak: 0
 48: 0 0
FindWordBreak: 0
 49: 0 1
FindWordBreak: 0
 50: 0 2
FindWordBreak: 0
 51: 0 3
FindWordBreak: 0
 52: 0 4
FindWordBreak: 0
 53: 0 5
FindWordBreak: 0
 54: 0 6
FindWordBreak: 0
 55: 0 7
FindWordBreak: 0
 56: 0 8
FindWordBreak: 0
 57: 0 9
FindWordBreak: 1
 58: 1 :
FindWordBreak: 1
 59: 1 ;
FindWordBreak: 1
 60: 1 <
FindWordBreak: 1
 61: 1 =
FindWordBreak: 1
 62: 1 >
FindWordBreak: 1
 63: 1 ?
FindWordBreak: 1
 64: 1 @
FindWordBreak: 0
 65: 0 A
FindWordBreak: 0
 66: 0 B
FindWordBreak: 0
 67: 0 C
FindWordBreak: 0
 68: 0 D
FindWordBreak: 0
 69: 0 E
FindWordBreak: 0
 70: 0 F
FindWordBreak: 0
 71: 0 G
FindWordBreak: 0
 72: 0 H
FindWordBreak: 0
 73: 0 I
FindWordBreak: 0
 74: 0 J
FindWordBreak: 0
 75: 0 K
FindWordBreak: 0
 76: 0 L
FindWordBreak: 0
 77: 0 M
FindWordBreak: 0
 78: 0 N
FindWordBreak: 0
 79: 0 O
FindWordBreak: 0
 80: 0 P
FindWordBreak: 0
 81: 0 Q
FindWordBreak: 0
 82: 0 R
FindWordBreak: 0
 83: 0 S
FindWordBreak: 0
 84: 0 T
FindWordBreak: 0
 85: 0 U
FindWordBreak: 0
 86: 0 V
FindWordBreak: 0
 87: 0 W
FindWordBreak: 0
 88: 0 X
FindWordBreak: 0
 89: 0 Y
FindWordBreak: 0
 90: 0 Z
FindWordBreak: 1
 91: 1 [
FindWordBreak: 1
 92: 1 \
FindWordBreak: 1
 93: 1 ]
FindWordBreak: 1
 94: 1 ^
FindWordBreak: 1
 95: 1 _
FindWordBreak: 1
 96: 1 `
FindWordBreak: 0
 97: 0 a
FindWordBreak: 0
 98: 0 b
FindWordBreak: 0
 99: 0 c
FindWordBreak: 0
100: 0 d
FindWordBreak: 0
101: 0 e
FindWordBreak: 0
102: 0 f
FindWordBreak: 0
103: 0 g
FindWordBreak: 0
104: 0 h
FindWordBreak: 0
105: 0 i
FindWordBreak: 0
106: 0 j
FindWordBreak: 0
107: 0 k
FindWordBreak: 0
108: 0 l
FindWordBreak: 0
109: 0 m
FindWordBreak: 0
110: 0 n
FindWordBreak: 0
111: 0 o
FindWordBreak: 0
112: 0 p
FindWordBreak: 0
113: 0 q
FindWordBreak: 0
114: 0 r
FindWordBreak: 0
115: 0 s
FindWordBreak: 0
116: 0 t
FindWordBreak: 0
117: 0 u
FindWordBreak: 0
118: 0 v
FindWordBreak: 0
119: 0 w
FindWordBreak: 0
120: 0 x
FindWordBreak: 0
121: 0 y
FindWordBreak: 0
122: 0 z
FindWordBreak: 1
123: 1 {
FindWordBreak: 1
124: 1 |
FindWordBreak: 1
125: 1 }
FindWordBreak: 1
126: 1 ~
FindWordBreak: 0
127: 0 
FindWordBreak: 0
128: 0 ?
FindWordBreak: 0
129: 0 Å
FindWordBreak: 1
130: 1 ?
FindWordBreak: 0
131: 0 ?
FindWordBreak: 1
132: 1 ?
FindWordBreak: 1
133: 1 ?
FindWordBreak: 1
134: 1 ?
FindWordBreak: 1
135: 1 ?
FindWordBreak: 0
136: 0 ?
FindWordBreak: 1
137: 1 ?
FindWordBreak: 0
138: 0 ?
FindWordBreak: 1
139: 1 ?
FindWordBreak: 0
140: 0 ?
FindWordBreak: 0
141: 0 ç
FindWordBreak: 0
142: 0 ?
FindWordBreak: 0
143: 0 è
FindWordBreak: 0
144: 0 ê
FindWordBreak: 1
145: 1 ?
FindWordBreak: 1
146: 1 ?
FindWordBreak: 1
147: 1 ?
FindWordBreak: 1
148: 1 ?
FindWordBreak: 1
149: 1 ?
FindWordBreak: 1
150: 1 ?
FindWordBreak: 1
151: 1 ?
FindWordBreak: 0
152: 0 ?
FindWordBreak: 0
153: 0 ?
FindWordBreak: 0
154: 0 ?
FindWordBreak: 1
155: 1 ?
FindWordBreak: 0
156: 0 ?
FindWordBreak: 0
157: 0 ù
FindWordBreak: 0
158: 0 ?
FindWordBreak: 0
159: 0 ?
FindWordBreak: 18
160: 18 †
FindWordBreak: 1
161: 1 °
FindWordBreak: 1
162: 1 ¢
FindWordBreak: 1
163: 1 £
FindWordBreak: 1
164: 1 §
FindWordBreak: 1
165: 1 •
FindWordBreak: 1
166: 1 ¶
FindWordBreak: 1
167: 1 ß
FindWordBreak: 1
168: 1 ®
FindWordBreak: 1
169: 1 ©
FindWordBreak: 1
170: 1 ™
FindWordBreak: 1
171: 1 ´
FindWordBreak: 1
172: 1 ¨
FindWordBreak: 1
173: 1 ≠
FindWordBreak: 1
174: 1 Æ
FindWordBreak: 1
175: 1 Ø
FindWordBreak: 1
176: 1 ∞
FindWordBreak: 1
177: 1 ±
FindWordBreak: 1
178: 1 ≤
FindWordBreak: 1
179: 1 ≥
FindWordBreak: 1
180: 1 ¥
FindWordBreak: 1
181: 1 µ
FindWordBreak: 1
182: 1 ∂
FindWordBreak: 1
183: 1 ∑
FindWordBreak: 1
184: 1 ∏
FindWordBreak: 1
185: 1 π
FindWordBreak: 1
186: 1 ∫
FindWordBreak: 1
187: 1 ª
FindWordBreak: 1
188: 1 º
FindWordBreak: 1
189: 1 Ω
FindWordBreak: 1
190: 1 æ
FindWordBreak: 1
191: 1 ø
FindWordBreak: 0
192: 0 ¿
FindWordBreak: 0
193: 0 ¡
FindWordBreak: 0
194: 0 ¬
FindWordBreak: 0
195: 0 √
FindWordBreak: 0
196: 0 ƒ
FindWordBreak: 0
197: 0 ≈
FindWordBreak: 0
198: 0 ∆
FindWordBreak: 0
199: 0 «
FindWordBreak: 0
200: 0 »
FindWordBreak: 0
201: 0 …
FindWordBreak: 0
202: 0  
FindWordBreak: 0
203: 0 À
FindWordBreak: 0
204: 0 Ã
FindWordBreak: 0
205: 0 Õ
FindWordBreak: 0
206: 0 Œ
FindWordBreak: 0
207: 0 œ
FindWordBreak: 0
208: 0 –
FindWordBreak: 0
209: 0 —
FindWordBreak: 0
210: 0 “
FindWordBreak: 0
211: 0 ”
FindWordBreak: 0
212: 0 ‘
FindWordBreak: 0
213: 0 ’
FindWordBreak: 0
214: 0 ÷
FindWordBreak: 1
215: 1 ◊
FindWordBreak: 0
216: 0 ÿ
FindWordBreak: 0
217: 0 Ÿ
FindWordBreak: 0
218: 0 ⁄
FindWordBreak: 0
219: 0 €
FindWordBreak: 0
220: 0 ‹
FindWordBreak: 0
221: 0 ›
FindWordBreak: 0
222: 0 ﬁ
FindWordBreak: 0
223: 0 ﬂ
FindWordBreak: 0
224: 0 ‡
FindWordBreak: 0
225: 0 ·
FindWordBreak: 0
226: 0 ‚
FindWordBreak: 0
227: 0 „
FindWordBreak: 0
228: 0 ‰
FindWordBreak: 0
229: 0 Â
FindWordBreak: 0
230: 0 Ê
FindWordBreak: 0
231: 0 Á
FindWordBreak: 0
232: 0 Ë
FindWordBreak: 0
233: 0 È
FindWordBreak: 0
234: 0 Í
FindWordBreak: 0
235: 0 Î
FindWordBreak: 0
236: 0 Ï
FindWordBreak: 0
237: 0 Ì
FindWordBreak: 0
238: 0 Ó
FindWordBreak: 0
239: 0 Ô
FindWordBreak: 0
240: 0 
FindWordBreak: 0
241: 0 Ò
FindWordBreak: 0
242: 0 Ú
FindWordBreak: 0
243: 0 Û
FindWordBreak: 0
244: 0 Ù
FindWordBreak: 0
245: 0 ı
FindWordBreak: 0
246: 0 ˆ
FindWordBreak: 1
247: 1 ˜
FindWordBreak: 0
248: 0 ¯
FindWordBreak: 0
249: 0 ˘
FindWordBreak: 0
250: 0 ˙
FindWordBreak: 0
251: 0 ˚
FindWordBreak: 0
252: 0 ¸
FindWordBreak: 0
253: 0 ˝
FindWordBreak: 0
254: 0 ˛
FindWordBreak: 0
255: 0 ˇ
*/
