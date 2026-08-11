/*
 * Jebiga-Gaming - Skindaten
 *
 * ERZEUGTE DATEI - NICHT VON HAND BEARBEITEN.
 * Erzeugt von tools/gen_data.py aus der Referenzdokumentation von open.mp
 * (docs/scripting/resources/skins.md). Aenderungen gehoeren in den Generator.
 *
 * 312 Skins, luekenlos von 0 bis 311.
 */

#if defined _jg_skins_inc
    #endinput
#endif
#define _jg_skins_inc


#define SKIN_COUNT              (312)

enum E_SKIN_INFO
{
    skName[44],
    bool:skFemale
};

new const gSkinInfo[SKIN_COUNT][E_SKIN_INFO] =
{
    {"Carl 'CJ' Johnson (Main Character)", false},   // 0
    {"The Truth", false},   // 1
    {"Maccer", false},   // 2
    {"Andre", false},   // 3
    {"Barry 'Big Bear' Thorne [Thin]", false},   // 4
    {"Barry 'Big Bear' Thorne [Big]", false},   // 5
    {"Emmet", false},   // 6
    {"Taxi Driver/Train Driver", false},   // 7
    {"Janitor", false},   // 8
    {"Normal Ped", true},   // 9
    {"Old Woman", true},   // 10
    {"Casino croupier", true},   // 11
    {"Rich Woman", true},   // 12
    {"Street Girl", true},   // 13
    {"Normal Ped", false},   // 14
    {"Mr.Whittaker (RS Haul Owner)", false},   // 15
    {"Airport Ground Worker", false},   // 16
    {"Businessman", false},   // 17
    {"Beach Visitor", false},   // 18
    {"DJ", false},   // 19
    {"Rich Guy (Madd Dogg's Manager)", false},   // 20
    {"Normal Ped", false},   // 21
    {"Normal Ped", false},   // 22
    {"BMXer", false},   // 23
    {"Madd Dogg Bodyguard", false},   // 24
    {"Madd Dogg Bodyguard", false},   // 25
    {"Backpacker", false},   // 26
    {"Construction Worker", false},   // 27
    {"Drug Dealer", false},   // 28
    {"Drug Dealer", false},   // 29
    {"Drug Dealer", false},   // 30
    {"Farm-Town inhabitant", true},   // 31
    {"Farm-Town inhabitant", false},   // 32
    {"Farm-Town inhabitant", false},   // 33
    {"Farm-Town inhabitant", false},   // 34
    {"Gardener", false},   // 35
    {"Golfer", false},   // 36
    {"Golfer", false},   // 37
    {"Normal Ped", true},   // 38
    {"Normal Ped", true},   // 39
    {"Normal Ped", true},   // 40
    {"Normal Ped", true},   // 41
    {"Jethro", false},   // 42
    {"Normal Ped", false},   // 43
    {"Normal Ped", false},   // 44
    {"Beach Visitor", false},   // 45
    {"Normal Ped", false},   // 46
    {"Normal Ped", false},   // 47
    {"Normal Ped", false},   // 48
    {"Snakehead (Da Nang)", false},   // 49
    {"Mechanic", false},   // 50
    {"Mountain Biker", false},   // 51
    {"Mountain Biker", false},   // 52
    {"Unknown", true},   // 53
    {"Normal Ped", true},   // 54
    {"Normal Ped", true},   // 55
    {"Normal Ped", true},   // 56
    {"Oriental Ped", false},   // 57
    {"Oriental Ped", false},   // 58
    {"Normal Ped", false},   // 59
    {"Normal Ped", false},   // 60
    {"Pilot", false},   // 61
    {"Colonel Fuhrberger", false},   // 62
    {"Prostitute", true},   // 63
    {"Prostitute", true},   // 64
    {"Kendl Johnson", true},   // 65
    {"Pool Player", false},   // 66
    {"Pool Player", false},   // 67
    {"Priest/Preacher", false},   // 68
    {"Normal Ped", true},   // 69
    {"Scientist", false},   // 70
    {"Security Guard", false},   // 71
    {"Hippy", false},   // 72
    {"Hippy", false},   // 73
    {"-", false},   // 74
    {"Prostitute", true},   // 75
    {"Stewardess", true},   // 76
    {"Homeless", true},   // 77
    {"Homeless", false},   // 78
    {"Homeless", false},   // 79
    {"Boxer", false},   // 80
    {"Boxer", false},   // 81
    {"Black Elvis", false},   // 82
    {"White Elvis", false},   // 83
    {"Blue Elvis", false},   // 84
    {"Prostitute", true},   // 85
    {"Ryder with robbery mask", false},   // 86
    {"Stripper", true},   // 87
    {"Normal Ped", true},   // 88
    {"Normal Ped", true},   // 89
    {"Jogger", true},   // 90
    {"Rich Woman", true},   // 91
    {"Rollerskater", true},   // 92
    {"Normal Ped", true},   // 93
    {"Normal Ped", false},   // 94
    {"Normal Ped", false},   // 95
    {"Jogger", false},   // 96
    {"Lifeguard", false},   // 97
    {"Normal Ped", false},   // 98
    {"Rollerskater", false},   // 99
    {"Biker", false},   // 100
    {"Normal Ped", false},   // 101
    {"Balla", false},   // 102
    {"Balla", false},   // 103
    {"Balla", false},   // 104
    {"Grove Street Families", false},   // 105
    {"Grove Street Families", false},   // 106
    {"Grove Street Families", false},   // 107
    {"Los Santos Vagos", false},   // 108
    {"Los Santos Vagos", false},   // 109
    {"Los Santos Vagos", false},   // 110
    {"The Russian Mafia", false},   // 111
    {"The Russian Mafia", false},   // 112
    {"The Russian Mafia", false},   // 113
    {"Varios Los Aztecas", false},   // 114
    {"Varios Los Aztecas", false},   // 115
    {"Varios Los Aztecas", false},   // 116
    {"Triad", false},   // 117
    {"Triad", false},   // 118
    {"Johhny Sindacco", false},   // 119
    {"Triad Boss", false},   // 120
    {"Da Nang Boy", false},   // 121
    {"Da Nang Boy", false},   // 122
    {"Da Nang Boy", false},   // 123
    {"The Mafia", false},   // 124
    {"The Mafia", false},   // 125
    {"The Mafia", false},   // 126
    {"The Mafia", false},   // 127
    {"Farm Inhabitant", false},   // 128
    {"Farm Inhabitant", true},   // 129
    {"Farm Inhabitant", true},   // 130
    {"Farm Inhabitant", true},   // 131
    {"Farm Inhabitant", false},   // 132
    {"Farm Inhabitant", false},   // 133
    {"Homeless", false},   // 134
    {"Homeless", false},   // 135
    {"Normal Ped", false},   // 136
    {"Homeless", false},   // 137
    {"Beach Visitor", true},   // 138
    {"Beach Visitor", true},   // 139
    {"Beach Visitor", true},   // 140
    {"Businesswoman", true},   // 141
    {"Taxi Driver", false},   // 142
    {"Crack Maker", false},   // 143
    {"Crack Maker", false},   // 144
    {"Crack Maker", true},   // 145
    {"Crack Maker", false},   // 146
    {"Businessman", false},   // 147
    {"Businesswoman", true},   // 148
    {"Big Smoke Armored", false},   // 149
    {"Businesswoman", true},   // 150
    {"Normal Ped", true},   // 151
    {"Prostitute", true},   // 152
    {"Construction Worker", false},   // 153
    {"Beach Visitor", false},   // 154
    {"Well Stacked Pizza Worker", false},   // 155
    {"Barber", false},   // 156
    {"Hillbilly", true},   // 157
    {"Farmer", false},   // 158
    {"Hillbilly", false},   // 159
    {"Hillbilly", false},   // 160
    {"Farmer", false},   // 161
    {"Hillbilly", false},   // 162
    {"Black Bouncer", false},   // 163
    {"White Bouncer", false},   // 164
    {"White MIB agent", false},   // 165
    {"Black MIB agent", false},   // 166
    {"Cluckin' Bell Worker", false},   // 167
    {"Hotdog/Chilli Dog Vendor", false},   // 168
    {"Normal Ped", true},   // 169
    {"Normal Ped", false},   // 170
    {"Blackjack Dealer", false},   // 171
    {"Casino croupier", true},   // 172
    {"San Fierro Rifa", false},   // 173
    {"San Fierro Rifa", false},   // 174
    {"San Fierro Rifa", false},   // 175
    {"Barber", false},   // 176
    {"Barber", false},   // 177
    {"Whore", true},   // 178
    {"Ammunation Salesman", false},   // 179
    {"Tattoo Artist", false},   // 180
    {"Punk", false},   // 181
    {"Cab Driver", false},   // 182
    {"Normal Ped", false},   // 183
    {"Normal Ped", false},   // 184
    {"Normal Ped", false},   // 185
    {"Normal Ped", false},   // 186
    {"Businessman", false},   // 187
    {"Normal Ped", false},   // 188
    {"Valet", false},   // 189
    {"Barbara Schternvart", true},   // 190
    {"Helena Wankstein", true},   // 191
    {"Michelle Cannes", true},   // 192
    {"Katie Zhan", true},   // 193
    {"Millie Perkins", true},   // 194
    {"Denise Robinson", true},   // 195
    {"Farm-Town inhabitant", true},   // 196
    {"Hillbilly", true},   // 197
    {"Farm-Town inhabitant", true},   // 198
    {"Farm-Town inhabitant", true},   // 199
    {"Hillbilly", false},   // 200
    {"Farmer", true},   // 201
    {"Farmer", false},   // 202
    {"Karate Teacher", false},   // 203
    {"Karate Teacher", false},   // 204
    {"Burger Shot Cashier", true},   // 205
    {"Cab Driver", false},   // 206
    {"Prostitute", true},   // 207
    {"Su Xi Mu (Suzie)", false},   // 208
    {"Oriental Noodle stand vendor", false},   // 209
    {"Oriental Boating School Instructor", false},   // 210
    {"Clothes shop staff", true},   // 211
    {"Homeless", false},   // 212
    {"Weird old man", false},   // 213
    {"Waitress (Maria Latore)", true},   // 214
    {"Normal Ped", true},   // 215
    {"Normal Ped", true},   // 216
    {"Clothes shop staff", false},   // 217
    {"Normal Ped", true},   // 218
    {"Rich Woman", true},   // 219
    {"Cab Driver", false},   // 220
    {"Normal Ped", false},   // 221
    {"Normal Ped", false},   // 222
    {"Normal Ped", false},   // 223
    {"Normal Ped", true},   // 224
    {"Normal Ped", true},   // 225
    {"Normal Ped", true},   // 226
    {"Oriental Businessman", false},   // 227
    {"Oriental Ped", false},   // 228
    {"Oriental Ped", false},   // 229
    {"Homeless", false},   // 230
    {"Normal Ped", true},   // 231
    {"Normal Ped", true},   // 232
    {"Normal Ped", true},   // 233
    {"Cab Driver", false},   // 234
    {"Normal Ped", false},   // 235
    {"Normal Ped", false},   // 236
    {"Prostitute", true},   // 237
    {"Prostitute", true},   // 238
    {"Homeless", false},   // 239
    {"The D.A", false},   // 240
    {"Afro-American", false},   // 241
    {"Mexican", false},   // 242
    {"Prostitute", true},   // 243
    {"Stripper", true},   // 244
    {"Prostitute", true},   // 245
    {"Stripper", true},   // 246
    {"Biker", false},   // 247
    {"Biker", false},   // 248
    {"Pimp", false},   // 249
    {"Normal Ped", false},   // 250
    {"Lifeguard", true},   // 251
    {"Naked Valet", false},   // 252
    {"Bus Driver", false},   // 253
    {"Biker Drug Dealer", false},   // 254
    {"Chauffeur (Limo Driver)", false},   // 255
    {"Stripper", true},   // 256
    {"Stripper", true},   // 257
    {"Heckler", false},   // 258
    {"Heckler", false},   // 259
    {"Construction Worker", false},   // 260
    {"Cab driver", false},   // 261
    {"Cab driver", false},   // 262
    {"Normal Ped", true},   // 263
    {"Clown (Ice-cream Van Driver)", false},   // 264
    {"Officer Frank Tenpenny (Corrupt Cop)", false},   // 265
    {"Officer Eddie Pulaski (Corrupt Cop)", false},   // 266
    {"Officer Jimmy Hernandez", false},   // 267
    {"Dwaine/Dwayne", false},   // 268
    {"Melvin 'Big Smoke' Harris (Mission)", false},   // 269
    {"Sean 'Sweet' Johnson", false},   // 270
    {"Lance 'Ryder' Wilson", false},   // 271
    {"Mafia Boss", false},   // 272
    {"T-Bone Mendez", false},   // 273
    {"Paramedic (Emergency Medical Technician)", false},   // 274
    {"Paramedic (Emergency Medical Technician)", false},   // 275
    {"Paramedic (Emergency Medical Technician)", false},   // 276
    {"Firefighter", false},   // 277
    {"Firefighter", false},   // 278
    {"Firefighter", false},   // 279
    {"Los Santos Police Officer", false},   // 280
    {"San Fierro Police Officer", false},   // 281
    {"Las Venturas Police Officer", false},   // 282
    {"County Sheriff", false},   // 283
    {"LSPD Motorbike Cop", false},   // 284
    {"S.W.A.T Special Forces", false},   // 285
    {"FBI Agent", false},   // 286
    {"San Andreas Army Soldier", false},   // 287
    {"Desert Sheriff", false},   // 288
    {"Zero", false},   // 289
    {"Ken Rosenberg", false},   // 290
    {"Kent Paul", false},   // 291
    {"Cesar Vialpando", false},   // 292
    {"Jeffery 'OG Loc' Martin/Cross", false},   // 293
    {"Wu Zi Mu (Woozie)", false},   // 294
    {"Michael Toreno (Mike)", false},   // 295
    {"Jizzy B.", false},   // 296
    {"Madd Dogg", false},   // 297
    {"Catalina", true},   // 298
    {"Claude Speed", false},   // 299
    {"Los Santos Police Officer (Without gun hols", false},   // 300
    {"San Fierro Police Officer (Without gun hols", false},   // 301
    {"Las Venturas Police Officer (Without gun ho", false},   // 302
    {"Los Santos Police Officer (Without uniform)", false},   // 303
    {"Los Santos Police Officer (Without uniform)", false},   // 304
    {"Las Venturas Police Officer (Without unifor", false},   // 305
    {"Los Santos Police Officer", true},   // 306
    {"San Fierro Police Officer", true},   // 307
    {"San Fierro Paramedic (Emergency Medical Tec", true},   // 308
    {"Las Venturas Police Officer", true},   // 309
    {"Country Sheriff (Without hat)", false},   // 310
    {"Desert Sheriff (Without hat)", false}   // 311
};

stock bool:IsValidSkin(skin)
{
    return (skin >= 0 && skin < SKIN_COUNT);
}

stock GetSkinName(skin)
{
    new name[44];
    if (!IsValidSkin(skin)) name = "Unbekannt";
    else format(name, sizeof name, "%s", gSkinInfo[skin][skName]);
    return name;
}

stock bool:IsFemaleSkin(skin)
{
    if (!IsValidSkin(skin)) return false;
    return gSkinInfo[skin][skFemale];
}
