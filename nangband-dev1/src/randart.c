/*
 * File: randart.c
 *
 * Abstract: Random artifact (randart) generator.
 *
 * Authors: Greg Wooledge, Ross Morgan-Linial, Andrew Sidwell.
 *
 * This file is dual-licenced under both the Angband and GPL licences.
 */

#include "angband.h"
#include "init.h"

/* Names list to build probabilities from. */
static cptr names_list =
"adanedhel\n"
"adurant\n"
"aeglos\n"
"aegnor\n"
"aelin\n"
"aeluin\n"
"aerandir\n"
"aerin\n"
"agarwaen\n"
"aglareb\n"
"aglarond\n"
"aglon\n"
"ainulindale\n"
"ainur\n"
"alcarinque\n"
"aldaron\n"
"aldudenie\n"
"almaren\n"
"alqualonde\n"
"aman\n"
"amandil\n"
"amarie\n"
"amarth\n"
"amlach\n"
"amon\n"
"amras\n"
"amrod\n"
"anach\n"
"anar\n"
"anarion\n"
"ancalagon\n"
"ancalimon\n"
"anarrima\n"
"andor\n"
"andram\n"
"androth\n"
"anduin\n"
"andunie\n"
"anfauglir\n"
"anfauglith\n"
"angainor\n"
"angband\n"
"anghabar\n"
"anglachel\n"
"angrenost\n"
"angrim\n"
"angrist\n"
"angrod\n"
"anguirel\n"
"annael\n"
"annatar\n"
"annon\n"
"annuminas\n"
"apanonar\n"
"aradan\n"
"aragorn\n"
"araman\n"
"aranel\n"
"aranruth\n"
"aranwe\n"
"aras\n"
"aratan\n"
"aratar\n"
"arathorn\n"
"arda\n"
"ard-galen\n"
"aredhel\n"
"ar-feiniel\n"
"argonath\n"
"arien\n"
"armenelos\n"
"arminas\n"
"arnor\n"
"aros\n"
"arossiach\n"
"arthad\n"
"arvernien\n"
"arwen\n"
"ascar\n"
"astaldo\n"
"atalante\n"
"atanamir\n"
"atanatari\n"
"atani\n"
"aule\n"
"avallone\n"
"avari\n"
"avathar\n"
"balan\n"
"balar\n"
"balrog\n"
"barad\n"
"baragund\n"
"barahir\n"
"baran\n"
"baranduin\n"
"bar\n"
"bauglir\n"
"beleg\n"
"belegaer\n"
"belegost\n"
"belegund\n"
"beleriand\n"
"belfalas\n"
"belthil\n"
"belthronding\n"
"beor\n"
"beraid\n"
"bereg\n"
"beren\n"
"boromir\n"
"boron\n"
"bragollach\n"
"brandir\n"
"bregolas\n"
"bregor\n"
"brethil\n"
"brilthor\n"
"brithiach\n"
"brithombar\n"
"brithon\n"
"cabed\n"
"calacirya\n"
"calaquendi\n"
"calenardhon\n"
"calion\n"
"camlost\n"
"caragdur\n"
"caranthir\n"
"carcharoth\n"
"cardolan\n"
"carnil\n"
"celeborn\n"
"celebrant\n"
"celebrimbor\n"
"celebrindal\n"
"celebros\n"
"celegorm\n"
"celon\n"
"cirdan\n"
"cirith\n"
"cirth\n"
"ciryatan\n"
"ciryon\n"
"coimas\n"
"corollaire\n"
"crissaegrim\n"
"cuarthal\n"
"cuivienen\n"
"culurien\n"
"curufin\n"
"curufinwe\n"
"curunir\n"
"cuthalion\n"
"daedeloth\n"
"daeron\n"
"dagnir\n"
"dagor\n"
"dagorlad\n"
"dairuin\n"
"danwedh\n"
"delduwath\n"
"denethor\n"
"dimbar\n"
"dimrost\n"
"dinen\n"
"dior\n"
"dirnen\n"
"dolmed\n"
"doriath\n"
"dorlas\n"
"dorthonion\n"
"draugluin\n"
"drengist\n"
"duath\n"
"duinath\n"
"duilwen\n"
"dunedain\n"
"dungortheb\n"
"earendil\n"
"earendur\n"
"earnil\n"
"earnur\n"
"earrame\n"
"earwen\n"
"echor\n"
"echoriath\n"
"ecthelion\n"
"edain\n"
"edrahil\n"
"eglador\n"
"eglarest\n"
"eglath\n"
"eilinel\n"
"eithel\n"
"ekkaia\n"
"elbereth\n"
"eldalie\n"
"eldalieva\n"
"eldamar\n"
"eldar\n"
"eledhwen\n"
"elemmire\n"
"elende\n"
"elendil\n"
"elendur\n"
"elenna\n"
"elentari\n"
"elenwe\n"
"elerrina\n"
"elleth\n"
"elmoth\n"
"elostirion\n"
"elrond\n"
"elros\n"
"elu\n"
"eluchil\n"
"elured\n"
"elurin\n"
"elwe\n"
"elwing\n"
"emeldir\n"
"endor\n"
"engrin\n"
"engwar\n"
"eol\n"
"eonwe\n"
"ephel\n"
"erchamion\n"
"ereb\n"
"ered\n"
"erech\n"
"eregion\n"
"ereinion\n"
"erellont\n"
"eressea\n"
"eriador\n"
"eru\n"
"esgalduin\n"
"este\n"
"estel\n"
"estolad\n"
"ethir\n"
"ezellohar\n"
"faelivrin\n"
"falas\n"
"falathar\n"
"falathrim\n"
"falmari\n"
"faroth\n"
"fauglith\n"
"feanor\n"
"feanturi\n"
"felagund\n"
"finarfin\n"
"finduilas\n"
"fingolfin\n"
"fingon\n"
"finwe\n"
"firimar\n"
"formenos\n"
"fornost\n"
"frodo\n"
"fuin\n"
"fuinur\n"
"gabilgathol\n"
"galad\n"
"galadriel\n"
"galathilion\n"
"galdor\n"
"galen\n"
"galvorn\n"
"gandalf\n"
"gaurhoth\n"
"gelion\n"
"gelmir\n"
"gelydh\n"
"gil\n"
"gildor\n"
"giliath\n"
"ginglith\n"
"girith\n"
"glaurung\n"
"glingal\n"
"glirhuin\n"
"gloredhel\n"
"glorfindel\n"
"golodhrim\n"
"gondolin\n"
"gondor\n"
"gonnhirrim\n"
"gorgoroth\n"
"gorlim\n"
"gorthaur\n"
"gorthol\n"
"gothmog\n"
"guilin\n"
"guinar\n"
"guldur\n"
"gundor\n"
"gurthang\n"
"gwaith\n"
"gwareth\n"
"gwindor\n"
"hadhodrond\n"
"hador\n"
"haladin\n"
"haldad\n"
"haldan\n"
"haldar\n"
"haldir\n"
"haleth\n"
"halmir\n"
"handir\n"
"harad\n"
"hareth\n"
"hathaldir\n"
"hathol\n"
"haudh\n"
"helcar\n"
"helcaraxe\n"
"helevorn\n"
"helluin\n"
"herumor\n"
"herunumen\n"
"hildorien\n"
"himlad\n"
"himring\n"
"hirilorn\n"
"hisilome\n"
"hithaeglir\n"
"hithlum\n"
"hollin\n"
"huan\n"
"hunthor\n"
"huor\n"
"hurin\n"
"hyarmendacil\n"
"hyarmentir\n"
"iant\n"
"iaur\n"
"ibun\n"
"idril\n"
"illuin\n"
"ilmare\n"
"ilmen\n"
"iluvatar\n"
"imlach\n"
"imladris\n"
"indis\n"
"ingwe\n"
"irmo\n"
"isil\n"
"isildur\n"
"istari\n"
"ithil\n"
"ivrin\n"
"kelvar\n"
"kementari\n"
"ladros\n"
"laiquendi\n"
"lalaith\n"
"lamath\n"
"lammoth\n"
"lanthir\n"
"laurelin\n"
"leithian\n"
"legolin\n"
"lembas\n"
"lenwe\n"
"linaewen\n"
"lindon\n"
"lindorie\n"
"loeg\n"
"lomelindi\n"
"lomin\n"
"lomion\n"
"lorellin\n"
"lorien\n"
"lorindol\n"
"losgar\n"
"lothlann\n"
"lothlorien\n"
"luin\n"
"luinil\n"
"lumbar\n"
"luthien\n"
"mablung\n"
"maedhros\n"
"maeglin\n"
"maglor\n"
"magor\n"
"mahanaxar\n"
"mahtan\n"
"maiar\n"
"malduin\n"
"malinalda\n"
"mandos\n"
"manwe\n"
"mardil\n"
"melian\n"
"melkor\n"
"menegroth\n"
"meneldil\n"
"menelmacar\n"
"meneltarma\n"
"minas\n"
"minastir\n"
"mindeb\n"
"mindolluin\n"
"mindon\n"
"minyatur\n"
"mirdain\n"
"miriel\n"
"mithlond\n"
"mithrandir\n"
"mithrim\n"
"mordor\n"
"morgoth\n"
"morgul\n"
"moria\n"
"moriquendi\n"
"mormegil\n"
"morwen\n"
"nahar\n"
"naeramarth\n"
"namo\n"
"nandor\n"
"nargothrond\n"
"narog\n"
"narsil\n"
"narsilion\n"
"narya\n"
"nauglamir\n"
"naugrim\n"
"ndengin\n"
"neithan\n"
"neldoreth\n"
"nenar\n"
"nenning\n"
"nenuial\n"
"nenya\n"
"nerdanel\n"
"nessa\n"
"nevrast\n"
"nibin\n"
"nienna\n"
"nienor\n"
"nimbrethil\n"
"nimloth\n"
"nimphelos\n"
"nimrais\n"
"nimras\n"
"ningloron\n"
"niniel\n"
"ninniach\n"
"ninquelote\n"
"niphredil\n"
"nirnaeth\n"
"nivrim\n"
"noegyth\n"
"nogrod\n"
"noldolante\n"
"noldor\n"
"numenor\n"
"nurtale\n"
"obel\n"
"ohtar\n"
"oiolosse\n"
"oiomure\n"
"olorin\n"
"olvar\n"
"olwe\n"
"ondolinde\n"
"orfalch\n"
"ormal\n"
"orocarni\n"
"orodreth\n"
"orodruin\n"
"orome\n"
"oromet\n"
"orthanc\n"
"osgiliath\n"
"osse\n"
"ossiriand\n"
"palantir\n"
"pelargir\n"
"pelori\n"
"periannath\n"
"quendi\n"
"quenta\n"
"quenya\n"
"radagast\n"
"radhruin\n"
"ragnor\n"
"ramdal\n"
"rana\n"
"rathloriel\n"
"rauros\n"
"region\n"
"rerir\n"
"rhovanion\n"
"rhudaur\n"
"rhun\n"
"rhunen\n"
"rian\n"
"ringil\n"
"ringwil\n"
"romenna\n"
"rudh\n"
"rumil\n"
"saeros\n"
"salmar\n"
"saruman\n"
"sauron\n"
"serech\n"
"seregon\n"
"serinde\n"
"shelob\n"
"silmarien\n"
"silmaril\n"
"silpion\n"
"sindar\n"
"singollo\n"
"sirion\n"
"soronume\n"
"sul\n"
"sulimo\n"
"talath\n"
"taniquetil\n"
"tar\n"
"taras\n"
"tarn\n"
"tathren\n"
"taur\n"
"tauron\n"
"teiglin\n"
"telchar\n"
"telemnar\n"
"teleri\n"
"telperion\n"
"telumendil\n"
"thalion\n"
"thalos\n"
"thangorodrim\n"
"thargelion\n"
"thingol\n"
"thoronath\n"
"thorondor\n"
"thranduil\n"
"thuringwethil\n"
"tilion\n"
"tintalle\n"
"tinuviel\n"
"tirion\n"
"tirith\n"
"tol\n"
"tulkas\n"
"tumhalad\n"
"tumladen\n"
"tuna\n"
"tuor\n"
"turambar\n"
"turgon\n"
"turin\n"
"uial\n"
"uilos\n"
"uinen\n"
"ulairi\n"
"ulmo\n"
"ulumuri\n"
"umanyar\n"
"umarth\n"
"umbar\n"
"ungoliant\n"
"urthel\n"
"uruloki\n"
"utumno\n"
"vaire\n"
"valacirca\n"
"valandil\n"
"valaquenta\n"
"valar\n"
"valaraukar\n"
"valaroma\n"
"valier\n"
"valimar\n"
"valinor\n"
"valinoreva\n"
"valmar\n"
"vana\n"
"vanyar\n"
"varda\n"
"vasa\n"
"vilya\n"
"vingilot\n"
"vinyamar\n"
"voronwe\n"
"wethrin\n"
"wilwarin\n"
"yavanna\n"
;

#define MAX_TRIES 200
#define BUFLEN 1024

#define MIN_NAME_LEN 5
#define MAX_NAME_LEN 9
#define S_WORD 26
#define E_WORD S_WORD

/* Find the sign of x */
#define sign(x)	((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

/* Add a resist to an item */
#define RANDART_RESIST(x_ptr, resist, min, max) \
	(x_ptr->resists[resist] = (rand_range(min, max)))

static unsigned short lprobs[S_WORD+1][S_WORD+1][S_WORD+1];
static unsigned short ltotal[S_WORD+1][S_WORD+1];

/* Hackish */
static bool probs_built = FALSE;

/*
 * Use W. Sheldon Simms' random name generator.  This function builds
 * probability tables which are used later on for letter selection.  It
 * relies on the ASCII character set.
 */
static void build_prob(cptr learn)
{
	int c_prev, c_cur, c_next;

	/* Build raw frequencies */
	do
	{
		c_prev = c_cur = S_WORD;

		do
		{
			c_next = *learn++;
		} while (!isalpha(c_next) && (c_next != '\0'));

		if (c_next == '\0') break;

		do
		{
			c_next = A2I(tolower(c_next));
			lprobs[c_prev][c_cur][c_next]++;
			ltotal[c_prev][c_cur]++;
			c_prev = c_cur;
			c_cur = c_next;
			c_next = *learn++;
		} while (isalpha(c_next));

		lprobs[c_prev][c_cur][E_WORD]++;
		ltotal[c_prev][c_cur]++;
	}
	while (c_next != '\0');
}

/*
 * Use W. Sheldon Simms' random name generator.  Generate a random word using
 * the probability tables we built earlier.  Relies on the ASCII character
 * set.  Relies on European vowels (a, e, i, o, u).  The generated name should
 * be copied/used before calling this function again.
 */
static char *make_word(void)
{
	static char word_buf[90];
	int r, totalfreq;
	int tries, lnum, vow;
	int c_prev, c_cur, c_next;
	char *cp;

	/* If we haven't built the probability tables, do so now */
	if (!probs_built)
	{
		build_prob(names_list);
		probs_built = TRUE;
	}

startover:
	vow = 0;
	lnum = 0;
	tries = 0;
	cp = word_buf;
	c_prev = c_cur = S_WORD;

	while (1)
	{
	    getletter:
		c_next = 0;
		r = rand_int(ltotal[c_prev][c_cur]);
		totalfreq = lprobs[c_prev][c_cur][c_next];
		while (totalfreq <= r)
		{
			c_next++;
			totalfreq += lprobs[c_prev][c_cur][c_next];
		}

		if (c_next == E_WORD)
		{
			if ((lnum < MIN_NAME_LEN) || vow == 0)
			{
				tries++;
				if (tries < 10) goto getletter;
				goto startover;
			}
			*cp = '\0';
			break;
		}
		if (lnum >= MAX_NAME_LEN) goto startover;

		*cp = c_next + 'a';	/* ASCII */
		switch (*cp)
		{
			case 'a': case 'e': case 'i': case 'o': case 'u':
				vow++;
		}

		cp++;
		lnum++;
		c_prev = c_cur;
		c_cur = c_next;
	}

	word_buf[0] = toupper(word_buf[0]);
	return word_buf;
}


/*
 * Use W. Sheldon Simms' random name generator.
 */
static void name_randart(object_type *o_ptr)
{
	char *word;

	do
	{
		word = make_word();
	} while (strlen(word) >= 17);

	if (rand_int(3) == 0)
		sprintf(o_ptr->bonuses->suffix_name, "'%s'", word);
	else
		sprintf(o_ptr->bonuses->suffix_name, "of %s", word);

	/* We are done. */
	return;
}


/*
 * Calculate the multiplier we'll get with a given bow type.
 */
static int bow_multiplier(int sval)
{
	switch (sval)
	{
		case SV_SLING:
		case SV_SHORT_BOW:
			return (2);
		case SV_LONG_BOW:
		case SV_LIGHT_XBOW:
			return (3);
		case SV_HEAVY_XBOW:
			return (4);
		default:
			msg_format("Illegal bow sval %s", sval);
	}

	return (0);
}

static s32b pval_table[9] = {0, 1, 2, 4, 6, 8, 10, 13, 10000};

/*
 * Evaluate the artifact's overall power level.
 */
s32b artifact_power(object_type *o_ptr)
{
	const randart_type *x_ptr = &x_info[o_ptr->name3];
	object_kind *k_ptr;
	const object_bonus *ob_ptr = o_ptr->bonuses;

	s32b p = 0, b;
	s16b k_idx;
	int i;

	/* Paranoia */
	if (!o_ptr->name3) return (0);

	/* Extract the object base kind */
	k_idx = o_ptr->k_idx;
	k_ptr = &k_info[k_idx];
	p = (k_ptr->level + 7) / 8;

	/* Evaluate certain abilities based on type of object. */
	switch (o_ptr->tval)
	{
		case TV_BOW:
		{
			int mult;

			p += (o_ptr->to_d + sign(o_ptr->to_d)) / 2;
			mult = bow_multiplier(o_ptr->sval);

			/* Get bonuses for might */
			if (x_ptr->flags1 & TR1_MIGHT)
			{
				if (o_ptr->pval > 3)
				{
					p += 20000;	/* inhibit */
					mult = 1;	/* don't overflow */
				}
				else
					mult += o_ptr->pval;
			}

			p *= mult;

			if (x_ptr->flags1 & TR1_SHOTS)
			{
				if (o_ptr->pval > 3)
					p += 20000;	/* inhibit */
				else if (o_ptr->pval > 0)
					p *= (2 * o_ptr->pval);
			}

			p += (o_ptr->to_h + 3 * sign(o_ptr->to_h)) / 4;
			if (o_ptr->weight < k_ptr->weight) p++;
			break;
		}
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		{
			p += (o_ptr->dd * (o_ptr->ds + 1)) / 5;
			p += (o_ptr->to_d + 2 * sign(o_ptr->to_d)) / 3;
			if (o_ptr->to_d > 15) p += (o_ptr->to_d - 14) / 2;

			b = p;

			if (x_ptr->flags1 & TR1_SLAY_EVIL)   p += b / 4;	/* + 50% dam, rarity factor 2 */
			if (x_ptr->flags1 & TR1_KILL_DRAGON) p += b / 3;	/* +200% dam, rarity factor 6 */
			if (x_ptr->flags1 & TR1_KILL_UNDEAD) p += b / 3;	/* +200% dam, rarity factor 6 */
			if (x_ptr->flags1 & TR1_KILL_DEMON)  p += b / 3;	/* +200% dam, rarity factor 6 */
			if (x_ptr->flags1 & TR1_SLAY_ANIMAL) p += b / 6;	/* + 50% dam, rarity factor 3 */
			if (x_ptr->flags1 & TR1_SLAY_UNDEAD) p += b / 4;	/* +100% dam, rarity factor 4 */
			if (x_ptr->flags1 & TR1_SLAY_DRAGON) p += b / 6;	/* +100% dam, rarity factor 6 */
			if (x_ptr->flags1 & TR1_SLAY_DEMON)  p += b / 6;	/* +100% dam, rarity factor 6 */
			if (x_ptr->flags1 & TR1_SLAY_TROLL)  p += b / 6;	/* +100% dam, rarity factor 6 */
			if (x_ptr->flags1 & TR1_SLAY_ORC)    p += b / 6;	/* +100% dam, rarity factor 6 */
			if (x_ptr->flags1 & TR1_SLAY_GIANT)  p += b / 9;	/* +100% dam, rarity factor 9 */

			if (x_ptr->flags2 & TR2_BRAND_NETHER) p += b / 2;	/* + 50% dam, rarity factor 1 */
			if (x_ptr->flags2 & TR2_BRAND_NEXUS)  p += b / 2;	/* + 50% dam, rarity factor 1 */
			if (x_ptr->flags2 & TR2_BRAND_CHAOS)  p += b / 2;	/* + 50% dam, rarity factor 1 */
			if (x_ptr->flags2 & TR2_BRAND_ACID)   p += b / 3;	/* + 50% dam, rarity factor 1.5 */
			if (x_ptr->flags2 & TR2_BRAND_ELEC)   p += b / 4;	/* + 50% dam, rarity factor 2 */
			if (x_ptr->flags2 & TR2_BRAND_FIRE)   p += b / 5;	/* + 50% dam, rarity factor 2.5 */
			if (x_ptr->flags2 & TR2_BRAND_COLD)   p += b / 4;	/* + 50% dam, rarity factor 2 */

			if (x_ptr->flags1 & TR1_BLOWS)
			{
				if (o_ptr->pval > 3)
					p += 20000;	/* inhibit */
				else if (o_ptr->pval > 0)
					p = (p * 6) / (4 - o_ptr->pval);
			}

			if ((x_ptr->flags1 & TR1_TUNNEL) &&
			    (o_ptr->tval != TV_DIGGING))
				p += o_ptr->pval * 3;

			p += (o_ptr->to_h + 3 * sign(o_ptr->to_h)) / 4;

			/* Remember, weight is in 0.1 lb. units. */
			if (o_ptr->weight != k_ptr->weight)
				p += (k_ptr->weight - o_ptr->weight) / 20;

			break;
		}
		case TV_BELT:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		{
			p += (o_ptr->ac + 4 * sign(o_ptr->ac)) / 5;
			p += (o_ptr->to_h + sign(o_ptr->to_h)) / 2;
			p += (o_ptr->to_d + sign(o_ptr->to_d)) / 2;
			if (o_ptr->weight != k_ptr->weight)
				p += (k_ptr->weight - o_ptr->weight) / 30;
			break;
		}
		case TV_LIGHT:
		{
			p += 10;
			break;
		}
		case TV_RING:
		case TV_AMULET:
		{
			p += 20;
			break;
		}
	}

	/* Other abilities are evaluated independent of the object type. */
	p += (o_ptr->to_a + 3 * sign(o_ptr->to_a)) / 4;
	if (o_ptr->to_a > 20) p += (o_ptr->to_a - 19) / 2;
	if (o_ptr->to_a > 30) p += (o_ptr->to_a - 29) / 2;
	if (o_ptr->to_a > 40) p += 20000;	/* inhibit */

	/* Do the stat bonuses */
	for (i = 0; i < A_MAX; i++)
	{
		int b = ob_ptr->stats[i];
		if (b > 8) b = 8;

		if (i == A_CHR) p += 3 * b;
		else
		{
			if (ob_ptr->stats[i] > 0) p += 3 * pval_table[b];
			else if (ob_ptr->stats[i] < 0) p += 2 * b;
		}
	}

	/* Use the pval ... */
	if (o_ptr->pval > 0)
	{
		if (x_ptr->flags1 & TR1_STEALTH) p += pval_table[((o_ptr->pval > 8) ? 8 : o_ptr->pval)];
	}
	else
	{
		if ((o_ptr->pval < 0) && (x_ptr->flags1 & TR1_STEALTH)) p += o_ptr->pval;
	}

	/* Apply nfravision bonuses */
	if (x_ptr->flags1 & TR1_INFRA) p += (o_ptr->pval + sign(o_ptr->pval));

	/* Apply speed bonuses */
	if (x_ptr->flags1 & TR1_SPEED)
	{
		p += 2 * o_ptr->pval;

		if (o_ptr->pval > 3) p += (o_ptr->pval - 3) * 3;
	}

	/* Apply sustain bonuses */
	if (x_ptr->flags2 & TR2_SUST_STR) p += 3;
	if (x_ptr->flags2 & TR2_SUST_INT) p += 2;
	if (x_ptr->flags2 & TR2_SUST_WIS) p += 2;
	if (x_ptr->flags2 & TR2_SUST_DEX) p += 2;
	if (x_ptr->flags2 & TR2_SUST_CON) p += 2;
	if (x_ptr->flags2 & TR2_SUST_CHR) p += 1;

	/* Do the resists */
	for (i = 0; i < RES_MAX; i++)
	{
		int amount = (x_ptr->resists[i] / 15);

		/* Reward high powers */
		if (x_ptr->resists[i] >= 80) amount += 10;
		if (x_ptr->resists[i] >= 90) amount += 10;

		/* Increase the power */
		p += amount;
	}

	/* Decide on power levels for the resists */
	if (x_ptr->resists[RES_ACID] >= 100) p += 20;
	else if (x_ptr->resists[RES_ACID] >= 50) p += 10;
	else if (x_ptr->resists[RES_ACID] >= 10) p += 3;

	if (x_ptr->resists[RES_ELEC] >= 100) p += 24;
	else if (x_ptr->resists[RES_ELEC] >= 50) p += 12;
	else if (x_ptr->resists[RES_ELEC] >= 10) p += 4;

	if (x_ptr->resists[RES_FIRE] >= 100) p += 36;
	else if (x_ptr->resists[RES_FIRE] >= 50) p += 18;
	else if (x_ptr->resists[RES_FIRE] >= 10) p += 5;

	if (x_ptr->resists[RES_COLD] >= 100) p += 24;
	else if (x_ptr->resists[RES_COLD] >= 50) p += 12;
	else if (x_ptr->resists[RES_COLD] >= 10) p += 4;

	if (x_ptr->resists[RES_DARK] >= 100) p += 16;
	else if (x_ptr->resists[RES_DARK] >= 50) p += 8;

/*	if (x_ptr->flags2 & TR2_RES_CONFU) p += 8;
	if (x_ptr->flags2 & TR2_RES_SOUND) p += 10;
	if (x_ptr->flags2 & TR2_RES_SHARD) p += 8;
	if (x_ptr->flags2 & TR2_RES_NETHR) p += 12;
	if (x_ptr->flags2 & TR2_RES_NEXUS) p += 10;
	if (x_ptr->flags2 & TR2_RES_CHAOS) p += 12; */
	if (x_ptr->flags2 & TR2_NO_BLIND) p += 10;
	if (x_ptr->flags2 & TR2_NO_DISENCHANT) p += 12;
	if (x_ptr->flags3 & TR3_FREE_ACT) p += 8;
	if (x_ptr->flags3 & TR3_HOLD_LIFE) p += 10;
	if (x_ptr->flags2 & TR2_LIGHT1) p += 3;
	if (x_ptr->flags2 & TR2_LIGHT2) p += 5;
	if (x_ptr->flags2 & TR2_LIGHT3) p += 9;
	if (x_ptr->flags2 & TR2_LIGHT4) p += 15;

	if (x_ptr->flags3 & TR3_FEATHER) p += 1;
	if (x_ptr->flags3 & TR3_SEE_INVIS) p += 4;
	if (x_ptr->flags3 & TR3_TELEPATHY) p += 12;
	if (x_ptr->flags3 & TR3_SLOW_DIGEST) p += 2;
	if (x_ptr->flags3 & TR3_REGEN) p += 6;
	if (x_ptr->flags3 & TR3_TELEPORT) p -= 12;
	if (x_ptr->flags3 & TR3_DRAIN_EXP) p -= 16;
	if (x_ptr->flags3 & TR3_AGGRAVATE) p -= 8;
	if (x_ptr->flags3 & TR3_BLESSED) p += 2;
	if (x_ptr->flags3 & TR3_LIGHT_CURSE) p -= 4;
	if (x_ptr->flags3 & TR3_HEAVY_CURSE) p -= 12;
/*	if (x_ptr->flags3 & TR3_PERMA_CURSE) p -= 36; */

	return (p);
}


/*
 * We've just added an ability which uses the pval bonus.  Make sure it's
 * not zero.  If it's currently negative, leave it negative (heh heh).
 */
static void do_pval(object_type *o_ptr)
{
	if (o_ptr->pval == 0)
	{
		o_ptr->pval = (s16b)(1 + rand_int(3));
	}
	else if (o_ptr->pval < 0)
	{
		if (rand_int(2) == 0) o_ptr->pval--;
	}
	else if (rand_int(3) > 0)
	{
		o_ptr->pval++;
	}

	/* We are done. */
	return;
}


/* -------------------------------------------- takkaria, 2002-04-24 ---
 * We've just added an stat bonus.  Make sure it's
 * not zero.  If it's currently negative, leave it negative (heh heh).
 * --------------------------------------------------------------------- */
static void do_statbonus(object_type *o_ptr, int i)
{
	object_bonus *ob_ptr = o_ptr->bonuses;

	if (ob_ptr->stats[i] == 0)
	{
		ob_ptr->stats[i] = 1 + rand_int(3);
	}
	else if (ob_ptr->stats[i] < 0)
	{
		if (rand_int(2) == 0) ob_ptr->stats[i]--;
	}
	else if (rand_int(3) > 0)
	{
		ob_ptr->stats[i]++;
	}

	/* We are done. */
	return;
}

/*
 * Remove contradictory flags.
 */
static void remove_contradictory(object_type *o_ptr)
{
	randart_type *x_ptr = &x_info[o_ptr->name3];
	const object_bonus *ob_ptr = o_ptr->bonuses;

	/* Aggravation removes stealth */
	if (x_ptr->flags3 & TR3_AGGRAVATE) x_ptr->flags1 &= ~(TR1_STEALTH);

	/* Prevent negative blows/shots/infravision */
	if (o_ptr->pval < 0)
	{
		x_ptr->flags1 &= ~(TR1_BLOWS);
		x_ptr->flags1 &= ~(TR1_SHOTS);
		x_ptr->flags1 &= ~(TR1_INFRA);
	}

	/* If a stat is damaged, no sustains */
	if (ob_ptr->stats[A_STR] < 0) x_ptr->flags2 &= ~(TR2_SUST_STR);
	if (ob_ptr->stats[A_INT] < 0) x_ptr->flags2 &= ~(TR2_SUST_INT);
	if (ob_ptr->stats[A_WIS] < 0) x_ptr->flags2 &= ~(TR2_SUST_WIS);
	if (ob_ptr->stats[A_DEX] < 0) x_ptr->flags2 &= ~(TR2_SUST_DEX);
	if (ob_ptr->stats[A_CON] < 0) x_ptr->flags2 &= ~(TR2_SUST_CON);
	if (ob_ptr->stats[A_CHR] < 0) x_ptr->flags2 &= ~(TR2_SUST_CHR);

	/* Execute removes slay */
	if (x_ptr->flags1 & TR1_KILL_DRAGON) x_ptr->flags1 &= ~(TR1_SLAY_DRAGON);
	if (x_ptr->flags1 & TR1_KILL_UNDEAD) x_ptr->flags1 &= ~(TR1_SLAY_UNDEAD);
	if (x_ptr->flags1 & TR1_KILL_DEMON)  x_ptr->flags1 &= ~(TR1_SLAY_DEMON);

	/* Draining XP and Holding life are mutually exclusive */
	if (x_ptr->flags3 & TR3_DRAIN_EXP) x_ptr->flags3 &= ~(TR3_HOLD_LIFE);

	/* Ammo cannot have any pval-affected flags, nor curses/ignores */
	if (o_ptr->tval == TV_SHOT || o_ptr->tval == TV_ARROW ||
		o_ptr->tval == TV_BOLT)
	{
		x_ptr->flags1 &= ~(TR1_PVAL_MASK);
		x_ptr->flags2 = 0;
		x_ptr->flags3 &= (TR3_IGNORE_MASK | TR3_CURSED_MASK);
	}

	/* We are done. */
	return;
}


/*
 * Randomly select an extra ability to be added to the artifact in question.
 * XXX - This function is way too large.
 *
 * ToDo: Add the KILL_UNDEAD and KILL_DEMON flags.
 */
static void add_ability(object_type *o_ptr, int power)
{
	randart_type *x_ptr = &x_info[o_ptr->name3];

	int r;
	int p = power;

	/* Boost power */
	while (rand_int(2) == 0)
		p += randint(10);

	/* Cap power */
	if (p > 50) p = 50;

	/*
	 *  0-29: minor ability
	 * 30-49: major ability
	 * 50+  : major power
	 */

	r = rand_int(p);
	if (rand_int(2) != 0)		/* Pick something dependent on item type. */
	{
		switch (o_ptr->tval)
		{
			case TV_BOLT:
			case TV_ARROW:
			case TV_SHOT:
			{
				if (r < 3)
					o_ptr->to_h += randint(5) + randint(5);
				else if (r < 6)
					o_ptr->to_d += randint(5) + randint(5);
				else if (r < 20)
				{
					o_ptr->to_h += randint(5) + randint(5);
					o_ptr->to_d += randint(5) + randint(5);
				}
				if (r < 30)
				{
					switch (rand_int(8))
					{
					case 0:
					case 1:
					case 2:
						{
							if (rand_int(2) == 0)
								x_ptr->flags1 |= TR1_SLAY_ORC;
							if (rand_int(2) == 0)
								x_ptr->flags1 |= TR1_SLAY_TROLL;
							if (rand_int(3) == 0)
								x_ptr->flags1 |= TR1_SLAY_GIANT;
							break;
						}
					case 3:
					case 4:
					case 5:
						{
							if (rand_int(2) == 0)
								x_ptr->flags1 |= TR1_SLAY_EVIL;
							if (rand_int(2) == 0)
								x_ptr->flags1 |= TR1_SLAY_UNDEAD;
							if (rand_int(3) == 0)
								x_ptr->flags1 |= TR1_SLAY_DEMON;
							break;
						}
					case 6:
						{
							x_ptr->flags1 |= TR1_SLAY_ANIMAL;
							break;
						}
					case 7:
						{
							x_ptr->flags1 |= TR1_SLAY_DRAGON;
							break;
						}
					}
				}
				else
				{
					switch (rand_int(10))
					{
					case 0:
						{
							x_ptr->flags1 |= TR1_KILL_DRAGON;
							break;
						}
					case 1:
						{
							x_ptr->flags1 |= TR1_KILL_DEMON;
							break;
						}
					case 2:
						{
							x_ptr->flags1 |= TR1_KILL_UNDEAD;
							break;
						}
					case 3:
						{
							x_ptr->flags1 |= TR2_BRAND_ACID;
							break;
						}
					case 4:
					case 5:
						{
							x_ptr->flags1 |= TR2_BRAND_ELEC;
							break;
						}
					case 6:
					case 7:
						{
							x_ptr->flags1 |= TR2_BRAND_FIRE;
							break;
						}
					case 8:
					case 9:
						{
							x_ptr->flags1 |= TR2_BRAND_COLD;
							break;
						}
					}
				}

				break;
			}
			case TV_BOW:
			{
				if (r < 5)
					o_ptr->to_h += randint(5) + randint(5);
				else if (r < 10)
					o_ptr->to_d += randint(5) + randint(5);
				else if (r < 30)
				{
					o_ptr->to_h += randint(5) + randint(5);
					o_ptr->to_d += randint(5) + randint(5);
				}
				else if (r < 33)
				{
                    do_statbonus(o_ptr, A_DEX);
					if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_DEX;
				}
				else if (rand_int(2) == 0)
				{
					x_ptr->flags1 |= TR1_SHOTS;
					do_pval(o_ptr);
				}
				else
				{
					x_ptr->flags1 |= TR1_MIGHT;
					do_pval(o_ptr);
				}
				break;
			}

			case TV_DIGGING:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			{
				if (r < 3)
				{
					o_ptr->to_h += randint(5) + randint(5);
				}
				else if (r < 6)
				{
					o_ptr->to_d += randint(5) + randint(5);
				}
				else if (r < 18)
				{
					o_ptr->to_h += randint(5) + randint(5);
					o_ptr->to_d += randint(5) + randint(5);
				}
				else if (r < 20 && o_ptr->tval != TV_HAFTED)
				{
					x_ptr->flags3 |= TR3_BLESSED;
				}
				else if (r < 28)
				{
					switch (rand_int(8))
					{
						case 0:
						case 1:
						case 2:
						{
							if (rand_int(2) == 0) x_ptr->flags1 |= TR1_SLAY_ORC;
							if (rand_int(2) == 0) x_ptr->flags1 |= TR1_SLAY_TROLL;
							if (rand_int(3) == 0) x_ptr->flags1 |= TR1_SLAY_GIANT;
							break;
						}

						case 3:
						case 4:
						case 5:
						{
							if (rand_int(2) == 0) x_ptr->flags1 |= TR1_SLAY_EVIL;
							if (rand_int(2) == 0) x_ptr->flags1 |= TR1_SLAY_UNDEAD;
							if (rand_int(3) == 0) x_ptr->flags1 |= TR1_SLAY_DEMON;
							if (o_ptr->tval != TV_HAFTED && rand_int(3) != 0)
							{
								x_ptr->flags3 |= TR3_BLESSED;
							}

							/* Maybe have a stat bonus */
							if (rand_int(p) >= 40)
							{
								do_statbonus(o_ptr, A_WIS);
								if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_WIS;
							}

							if (rand_int(5) == 0) x_ptr->flags3 |= TR3_HOLD_LIFE;

							break;
						}

						case 6:
						{
							x_ptr->flags1 |= TR1_SLAY_ANIMAL;

							if (rand_int(p) >= 40)
							{
								do_statbonus(o_ptr, A_INT);
								if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_INT;
							}

							if (rand_int(5) == 0) x_ptr->flags3 |= TR3_REGEN;

							break;
						}

						case 7:
						{
							x_ptr->flags1 |= TR1_SLAY_DRAGON;

							if (rand_int(p) >= 40)
							{
								do_statbonus(o_ptr, A_CON);
								if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_CON;
							}

							break;
						}
					}
				}
				else if (r < 30)
					o_ptr->to_a += randint(5);
				else if (r < 32)
					o_ptr->dd++;
				else if (r < 40)
				{
					switch (rand_int(8))
					{
					case 0:
						{
							x_ptr->flags1 |= TR1_KILL_DRAGON;
							if (rand_int(p) >= 40)
							{
               			     do_statbonus(o_ptr, A_CON);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_CON;

							}
							break;
						}
					case 1:
						{
							x_ptr->flags1 |= TR2_BRAND_ACID;
							if (rand_int(p) >= 20)

							break;
						}
					case 2:
					case 3:
						{
							x_ptr->flags1 |= TR2_BRAND_ELEC;
							if (rand_int(p) >= 20)
								RANDART_RESIST(x_ptr, RES_ELEC, 25, 35);
							break;
						}
					case 4:
					case 5:
						{
							x_ptr->flags1 |= TR2_BRAND_FIRE;
							if (rand_int(p) >= 20)
								RANDART_RESIST(x_ptr, RES_FIRE, 25, 35);
							break;
						}
					case 6:
					case 7:
						{
							x_ptr->flags1 |= TR2_BRAND_COLD;
							if (rand_int(p) >= 20)
								RANDART_RESIST(x_ptr, RES_COLD, 25, 35);
							break;
						}
					}
				}
				else if (r < 42)
				{
					switch (rand_int(3))
					{
					case 0:
						{
							x_ptr->flags2 |= TR2_BRAND_NEXUS;
							if (rand_int(p) >= 30)
								RANDART_RESIST(x_ptr, RES_NEXUS, 25, 35);
							break;
						}
					case 1:
						{
							x_ptr->flags2 |= TR2_BRAND_NETHER;
							if (rand_int(p) >= 30)
								RANDART_RESIST(x_ptr, RES_NETHER, 25, 35);
							break;
						}
					case 2:
						{
							x_ptr->flags2 |= TR2_BRAND_CHAOS;
							if (rand_int(p) >= 30)

							break;
						}
					}
				}
				else if (r < 44)
				{
               			     do_statbonus(o_ptr, A_STR);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_STR;
				}
				else if (r < 47)
				{
               			     do_statbonus(o_ptr, A_DEX);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_DEX;
				}
				else
				{
               			     do_statbonus(o_ptr, A_CON);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_CON;
				}

				break;
			}
			case TV_BOOTS:
			{
				if (r < 6)
					x_ptr->flags3 |= TR3_FEATHER;
				else if (r < 25)
					o_ptr->to_a += randint(5) + randint(5);
				else if (r < 30)
				{
					x_ptr->flags1 |= TR1_STEALTH;
					do_pval(o_ptr);
				}
				else if (r < 35)
				{
               			     do_statbonus(o_ptr, A_DEX);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_DEX;
				}
				else if (r < 45)
					x_ptr->flags3 |= TR3_FREE_ACT;
				else
				{
					x_ptr->flags1 |= TR1_SPEED;
					do_pval(o_ptr);

					/* Supercharge! */
					while (o_ptr->pval > 0 && rand_int(3) == 0)
						o_ptr->pval++;
				}

				break;
			}
			case TV_GLOVES:
			{
				if (r < 25)
					o_ptr->to_a += randint(5) + randint(5);
				else if (r < 30)
				{
					o_ptr->to_h += rand_int(6);
					o_ptr->to_d += rand_int(6);
				}
				else if (r < 40)
				{
               			     do_statbonus(o_ptr, A_DEX);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_DEX;
				}
				else
					x_ptr->flags3 |= TR3_FREE_ACT;

 				break;
 			}
 			case TV_HELM:
 			case TV_CROWN:
 			{
				if (r < 13)
					o_ptr->to_a += randint(5) + randint(5);
				else if (r < 15)
				{
					if (rand_int(5) == 0)
					{
						if (rand_int(20) == 0)
						{
							x_ptr->flags2 |= TR2_LIGHT3;
						}
						else
						{
							x_ptr->flags2 |= TR2_LIGHT2;
						}
					}
					else
					{
						x_ptr->flags2 |= TR2_LIGHT1;
					}
				}
				else if (r < 19)
				{
					x_ptr->flags1 |= TR1_INFRA;
					do_pval(o_ptr);
				}
				else if (r < 22)
				{
					x_ptr->flags1 |= TR1_SEARCH;
					do_pval(o_ptr);
				}
				else if (r < 26)
					x_ptr->flags3 |= TR3_SEE_INVIS;
				else if (r < 30)
				{
               			     do_statbonus(o_ptr, A_CHR);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_CHR;
				}
				else if (r < 37)
				{
               			     do_statbonus(o_ptr, A_INT);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_INT;
				}
				else if (r < 44)
				{
               			     do_statbonus(o_ptr, A_WIS);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_WIS;
				}
				else
					x_ptr->flags3 |= TR3_TELEPATHY;

 				break;
 			}
 			case TV_SHIELD:
 			{
				if (r < 20)
					o_ptr->to_a += randint(5) + randint(5);

				else if (r < 30)
				{
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_ACID, 15, 30);
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_ELEC, 15, 30);
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_FIRE, 15, 30);
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_COLD, 15, 30);
				}
				else if (r < 34)
				{
					RANDART_RESIST(x_ptr, RES_ACID, 20, 30);
					RANDART_RESIST(x_ptr, RES_ELEC, 20, 30);
					RANDART_RESIST(x_ptr, RES_FIRE, 20, 30);
					RANDART_RESIST(x_ptr, RES_COLD, 20, 30);
				}
				else if (r < 47)
				{
					switch (rand_int(11))
					{
					case 0:  RANDART_RESIST(x_ptr, RES_POIS, 20, 30); break;
					case 1:  RANDART_RESIST(x_ptr, RES_DOOM, 20, 30); break;
					case 2:  RANDART_RESIST(x_ptr, RES_DARK, 20, 30); break;
					case 3:  RANDART_RESIST(x_ptr, RES_CONF, 20, 30); break;
					case 4:  RANDART_RESIST(x_ptr, RES_SOUND, 20, 30); break;
					case 5:  RANDART_RESIST(x_ptr, RES_SHARDS, 20, 30); break;
					case 6:  RANDART_RESIST(x_ptr, RES_NETHER, 20, 30); break;
					case 7:  RANDART_RESIST(x_ptr, RES_NEXUS, 20, 30); break;
					case 8:  RANDART_RESIST(x_ptr, RES_CHAOS, 20, 30); break;
					case 9:  x_ptr->flags2 |= TR2_NO_BLIND; break;
					case 10: x_ptr->flags2 |= TR2_NO_DISENCHANT; break;
					}
				}
				else
					x_ptr->flags3 |= TR3_HOLD_LIFE;

 				break;
 			}
 			case TV_CLOAK:
 			{
				if (r < 20)
					o_ptr->to_a += randint(5) + randint(5);
				else if (r < 30)
				{
					x_ptr->flags1 |= TR1_STEALTH;
					do_pval(o_ptr);
				}
				else if (r < 44)
				{
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_ACID, 20, 30);
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_ELEC, 20, 30);
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_FIRE, 20, 30);
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_COLD, 20, 30);
 				}
				else if (r < 47)
					x_ptr->flags3 |= TR3_FREE_ACT;
				else
					x_ptr->flags3 |= TR3_HOLD_LIFE;

 				break;
 			}
			case TV_DRAG_ARMOR:
 			case TV_SOFT_ARMOR:
 			case TV_HARD_ARMOR:
 			{
				if (r < 25)
					o_ptr->to_a += randint(5) + randint(5);
				else if (r < 30)
				{
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_ACID, 20, 30);
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_ELEC, 20, 30);
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_FIRE, 20, 30);
					if (rand_int(3) == 0)
						RANDART_RESIST(x_ptr, RES_COLD, 20, 30);
				}
				else if (r < 34)
				{
					RANDART_RESIST(x_ptr, RES_ACID, 20, 30);
					RANDART_RESIST(x_ptr, RES_ELEC, 20, 30);
					RANDART_RESIST(x_ptr, RES_FIRE, 20, 30);
					RANDART_RESIST(x_ptr, RES_COLD, 20, 30);
 				}
				else if (r < 47)
				{
					switch (rand_int(11))
					{
					case 0:  RANDART_RESIST(x_ptr, RES_POIS, 20, 30); break;
					case 1:  RANDART_RESIST(x_ptr, RES_DOOM, 20, 30); break;
					case 2:  RANDART_RESIST(x_ptr, RES_DARK, 20, 30); break;
					case 3:  x_ptr->flags2 |= TR2_NO_BLIND; break;
					case 4:  RANDART_RESIST(x_ptr, RES_CONF, 20, 30); break;
					case 5:  RANDART_RESIST(x_ptr, RES_SOUND, 20, 30); break;
					case 6:  RANDART_RESIST(x_ptr, RES_SHARDS, 20, 30); break;
					case 7:  RANDART_RESIST(x_ptr, RES_NETHER, 20, 30); break;
					case 8:  RANDART_RESIST(x_ptr, RES_NEXUS, 20, 30); break;
					case 9:  RANDART_RESIST(x_ptr, RES_CHAOS, 20, 30); break;
					case 10: x_ptr->flags2 |= TR2_NO_DISENCHANT; break;
					}
				}
				else
					x_ptr->flags3 |= TR3_HOLD_LIFE;

 				break;
 			}
 		}
 	}
 	else			/* Pick something universally useful. */
 	{
		r = rand_int(power);

		if (r < 2)
			x_ptr->flags2 |= TR2_LIGHT1;
		else if (r < 4)
			x_ptr->flags3 |= TR3_FEATHER;
		else if (r < 7)
			x_ptr->flags3 |= TR3_SLOW_DIGEST;
		else if (r < 10)
			x_ptr->flags3 |= TR3_SEE_INVIS;
		else if (r < 12)
		{
			switch (rand_int(6))
 			{
			case 0: x_ptr->flags2 |= TR2_SUST_STR; break;
			case 1: x_ptr->flags2 |= TR2_SUST_INT; break;
			case 2: x_ptr->flags2 |= TR2_SUST_WIS; break;
			case 3: x_ptr->flags2 |= TR2_SUST_DEX; break;
			case 4: x_ptr->flags2 |= TR2_SUST_CON; break;
			case 5: x_ptr->flags2 |= TR2_SUST_CHR; break;
			}
		}
		else if (r < 13)
		{
			x_ptr->flags1 |= TR1_INFRA;
			do_pval(o_ptr);
		}
		else if (r < 14)
		{
			x_ptr->flags1 |= TR1_SEARCH;
			do_pval(o_ptr);
		}
		else if (r < 15)
		{
			x_ptr->flags1 |= TR1_STEALTH;
			do_pval(o_ptr);
		}
		else if (r < 19)
		{
			switch (rand_int(4))
			{
			case 0: RANDART_RESIST(x_ptr, RES_ACID, 20, 30); break;
			case 1: RANDART_RESIST(x_ptr, RES_ELEC, 20, 30); break;
			case 2: RANDART_RESIST(x_ptr, RES_FIRE, 20, 30); break;
			case 3: RANDART_RESIST(x_ptr, RES_COLD, 20, 30); break;
			}
		}
		else if (r < 21)
		{
               			     do_statbonus(o_ptr, A_CHR);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_CHR;
		}
		else if (r < 23)
		{

               			     do_statbonus(o_ptr, A_DEX);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_DEX;
		}
		else if (r < 25)
		{

               			     do_statbonus(o_ptr, A_INT);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_INT;
		}
		else if (r < 27)
		{

               			     do_statbonus(o_ptr, A_WIS);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_WIS;
		}
		else if (r < 29)
		{

               			     do_statbonus(o_ptr, A_CON);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_CON;
		}
		else if (r < 31)
		{

               			     do_statbonus(o_ptr, A_STR);
					    	 if (rand_int(3) == 0) x_ptr->flags2 |= TR2_SUST_STR;
		}
		else if (r < 33)
			x_ptr->flags3 |= TR3_REGEN;
		else if (r < 34)
			x_ptr->flags3 |= TR3_TELEPATHY;
		else if (r < 38)
		{
			switch (rand_int(11))
 			{
			case 0:  RANDART_RESIST(x_ptr, RES_POIS, 20, 30); break;
			case 1:  RANDART_RESIST(x_ptr, RES_DOOM, 20, 30); break;
			case 2:  RANDART_RESIST(x_ptr, RES_DARK, 20, 30); break;
			case 3:  x_ptr->flags2 |= TR2_NO_BLIND; break;
			case 4:  RANDART_RESIST(x_ptr, RES_CONF, 20, 30); break;
			case 5:  RANDART_RESIST(x_ptr, RES_SOUND, 20, 30); break;
			case 6:  RANDART_RESIST(x_ptr, RES_SHARDS, 20, 30); break;
			case 7:  RANDART_RESIST(x_ptr, RES_NETHER, 20, 30); break;
			case 8:  RANDART_RESIST(x_ptr, RES_NEXUS, 20, 30); break;
			case 9:  RANDART_RESIST(x_ptr, RES_CHAOS, 20, 30); break;
			case 10: x_ptr->flags2 |= TR2_NO_DISENCHANT; break;
			}
		}
		else if (r < 41)
			x_ptr->flags3 |= TR3_FREE_ACT;
		else if (r < 44)
			x_ptr->flags3 |= TR3_HOLD_LIFE;
		else if (r < 48)
		{
			x_ptr->flags1 |= TR1_SPEED;
			do_pval(o_ptr);

			/* Supercharge! */
			while ((o_ptr->pval > 0) && (rand_int(3) == 0))
				o_ptr->pval++;
		}
		else
		{
			switch (rand_int(4))
			{
			case 0: RANDART_RESIST(x_ptr, RES_ACID, 30, 50); break;
			case 1: RANDART_RESIST(x_ptr, RES_ELEC, 30, 50); break;
			case 2: RANDART_RESIST(x_ptr, RES_FIRE, 30, 50); break;
			case 3: RANDART_RESIST(x_ptr, RES_COLD, 30, 50); break;
 			}

 		}
 	}

 	/* Now remove contradictory or redundant powers. */
	remove_contradictory(o_ptr);

	/* We are done */
	return;
}


/*
 * Make it bad, or if it's already bad, make it worse!
 */
static void do_curse(object_type *o_ptr)
{
	randart_type *x_ptr = &x_info[o_ptr->name3];

	if (rand_int(3) == 0)
		x_ptr->flags3 |= TR3_AGGRAVATE;
	if (rand_int(5) == 0)
		x_ptr->flags3 |= TR3_DRAIN_EXP;
	if (rand_int(7) == 0)
		x_ptr->flags3 |= TR3_TELEPORT;

	if ((o_ptr->pval > 0) && (rand_int(2) == 0))
		o_ptr->pval = -o_ptr->pval;
	if ((o_ptr->to_a > 0) && (rand_int(2) == 0))
		o_ptr->to_a = -o_ptr->to_a;
	if ((o_ptr->to_h > 0) && (rand_int(2) == 0))
		o_ptr->to_h = -o_ptr->to_h;
	if ((o_ptr->to_d > 0) && (rand_int(4) == 0))
		o_ptr->to_d = -o_ptr->to_d;

	if (x_ptr->flags3 & TR3_LIGHT_CURSE)
	{
		if (rand_int(2) == 0) x_ptr->flags3 |= TR3_HEAVY_CURSE;
		return;
	}

	x_ptr->flags3 |= TR3_LIGHT_CURSE;

	if (rand_int(4) == 0)
		x_ptr->flags3 |= TR3_HEAVY_CURSE;
}

static byte activation_table[60] = {
	ACT_ILLUMINATION,
	ACT_MISSILE,
	ACT_PHASE,
	ACT_CURE_WOUNDS,
	ACT_FIRE1,
	ACT_STINKING_CLOUD,
	ACT_FROST1,
	ACT_ILLUMINATION,
	ACT_MISSILE,
	ACT_CURE_WOUNDS,
	ACT_TRAP_DOOR_DEST,
	ACT_SLEEP,
	ACT_STONE_TO_MUD,
	ACT_FIRE1,
	ACT_ILLUMINATION,
	ACT_STINKING_CLOUD,
	ACT_PHASE,
	ACT_MISSILE,
	ACT_FROST2,
	ACT_ACID1,
	ACT_PROBE,
	ACT_REM_FEAR_POIS,
	ACT_CONFUSE,
	ACT_MAGIC_MAP,
	ACT_TELEPORT,
	ACT_MAGIC_MAP,
	ACT_HEAL1,
	ACT_HASTE1,
	ACT_DETECT,
	ACT_IDENTIFY,
	ACT_RESIST,
	ACT_FIRE2,
	ACT_FROST3,
	ACT_TELEPORT,
	ACT_FROST4,
	ACT_ELEC2,
	ACT_FIRE3,
	ACT_TELEPORT,
	ACT_IDENTIFY,
	ACT_RECHARGE1,
	ACT_DRAIN_LIFE1,
	ACT_PROT_EVIL,
	ACT_LIGHTNING_BOLT,
	ACT_DETECT,
	ACT_RESIST,
	ACT_TELE_AWAY,
	ACT_ARROW,
	ACT_WOR,
	ACT_HASTE2,
	ACT_HEAL2,
	ACT_RAGE_BLESS_RESIST,
	ACT_FROST5,
	ACT_DISP_EVIL,
	ACT_WOR,
	ACT_FIREBRAND,
	ACT_DRAIN_LIFE2,
	ACT_STAR_BALL,
	ACT_RESTORE_LIFE,
	ACT_GENOCIDE,
	ACT_MASS_GENOCIDE,
};

bool make_randart_stupid(object_type *o_ptr)
{
	int i;
	int x_idx = 0;
	randart_type *x_ptr;

	for (i = 1; i < z_info->randart_max; i++)
	{
		if (x_info[i].name[0]) continue;
		x_idx = i;
		break;
	}

	if (!x_idx) return (FALSE);

	object_make_bonuses(o_ptr);

	o_ptr->name3 = x_idx;

	x_ptr = &x_info[x_idx];

	x_ptr->flags1 = 0;
	x_ptr->flags2 = 0;
	x_ptr->flags3 = 0;

	name_randart(o_ptr);

	for (i = 0; i < 4; i++)
		add_ability(o_ptr, 30);
	for (i = 0; i < 2; i++)
		add_ability(o_ptr, 50);

	/*
	 * Add TR3_HIDE_TYPE to all artifacts with nonzero pval because we're
	 * too lazy to find out which ones need it and which ones don't.
	 */
	if (o_ptr->pval)
		x_ptr->flags3 |= TR3_HIDE_TYPE;

	return (TRUE);

}

bool make_randart(object_type *o_ptr, bool curse)
{
	randart_type *x_ptr;
	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	int x_idx = 0;
	s32b power;
	int tries;
	s32b ap;
	bool aggravate_me = FALSE;
	int i;
	u32b f1, f2, f3;

	/* Find a free index */
	for (i = 1; i < z_info->randart_max; i++) 
	{
		if (x_info[i].name[0]) continue;
		x_idx = i;
		break;
	}

	if (!x_idx) return (FALSE);

	/* Make sure everything's set up */
	object_make_bonuses(o_ptr);

	/* Get the object's current flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Get the artifact */
	x_ptr = &x_info[x_idx];

	/* Clear the artifact */
	x_ptr->cost = k_ptr->cost;
	x_ptr->flags1 = f1;
	x_ptr->flags2 = f2;
	x_ptr->flags3 = f3 & ~(TR3_ACTIVATE);
	x_ptr->level = (p_ptr->depth + k_ptr->level) / 2;
	x_ptr->activation = 0;
	x_ptr->time = 0;
	x_ptr->randtime = 0;

	/* Name it */
	name_randart(o_ptr);

	/* Assign the artifact index */
	o_ptr->name3 = x_idx;

	/* Sometimes artifacts are deeper */
	while (rand_int((x_ptr->level - (p_ptr->depth + k_ptr->level) / 2) * 4) == 0)
		x_ptr->level++;

	/* Get current power */
	ap = artifact_power(o_ptr);

	/* Find a power */
	power = (ap + 30) / 2 + randint(x_ptr->level + 10);

	/* Reduce powerful items */
	if (power > ap + 30)
		power = ap + 30 + randint(power - ap - 30);

	/* Always add something */
	if (power < ap + 10) power = ap + 10;

	/* Find a power based on level and rarity */
	if (curse)
		power = -power;

	/* Sometimes add an activation */
	if (rand_int(200) < power)
	{
		int m = x_ptr->level / 2 + 20;

		/* Inhibit */
		if (m > 60) m = 60;

		x_ptr->flags3 |= TR3_ACTIVATE;
		x_ptr->activation = activation_table[rand_int(m)];

		/* Default values */
		x_ptr->time = x_ptr->randtime = 2 + x_ptr->level * x_ptr->level / 4;

		/* Find a real artifact with the right activation */
		for (i = 0; i < z_info->a_max; i++)
		{
			if (a_info[i].tval == 0) continue;
			if (!(a_info[i].flags3 & TR3_ACTIVATE)) continue;
			if (a_info[i].activation != x_ptr->activation) continue;

			/* Base on that timeout */
			x_ptr->time = a_info[i].time * a_info[i].level / x_ptr->level;
			x_ptr->randtime = a_info[i].randtime * a_info[i].level / x_ptr->level;

			/* Not too much worse */
			if (x_ptr->time > a_info[i].time * 2) x_ptr->time = a_info[i].time * 2;
			if (x_ptr->randtime > a_info[i].randtime * 2) x_ptr->randtime = a_info[i].randtime * 2;

			break;
		}

		/* Ensure sanity */
		if (x_ptr->time < 2) x_ptr->time = 2;
	}

	/* Really powerful items should aggravate. */
	if (power > 60)
	{
		if (rand_int(60) < (power - 60))
			aggravate_me = TRUE;
	}

	/* First draft: add two abilities, then curse it three times. */
	if (curse)
	{
		add_ability(o_ptr, 20);
		add_ability(o_ptr, 20);
		do_curse(o_ptr);
		do_curse(o_ptr);
		do_curse(o_ptr);
		remove_contradictory(o_ptr);
		ap = artifact_power(o_ptr);
	}
	else
	{
		/*
		 * Select a random set of abilities which roughly matches the
		 * original's in terms of overall power/usefulness.
		 */
		for (tries = 0; tries < MAX_TRIES; tries++)
		{
			object_type o_old;
			randart_type x_old;

			/* Copy artifact info temporarily. */
			o_old = *o_ptr;
			x_old = *x_ptr;
			add_ability(o_ptr, 20 + randint(power * 2 / 3));
			ap = artifact_power(o_ptr);

			if (ap > (power * 11) / 10 + 1)
			{
				/* Too powerful */
				*o_ptr = o_old;
				*x_ptr = x_old;

				continue;
			}
			else if (ap >= (power * 9) / 10)	/* just right */
			{
				break;
			}

			/* Stop if we're going negative, so we don't overload
			   the artifact with great powers to compensate. */
			else if ((ap < 0) && (ap < (-(power * 5)) / 10))
			{
				break;
			}
		}		/* end of power selection */

		if (aggravate_me)
		{
			x_ptr->flags3 |= TR3_AGGRAVATE;
			remove_contradictory(o_ptr);
			ap = artifact_power(o_ptr);
		}
	}

	/* Work out the cost */
	x_ptr->cost = ap * (s32b)(400 + 100 * randint(6));

	/* Restore the cost */
	if (x_ptr->cost < 0) x_ptr->cost = 0;

	/*
	 * Add TR3_HIDE_TYPE to all artifacts with nonzero pval because we're
	 * too lazy to find out which ones need it and which ones don't.
	 */
	if (o_ptr->pval)
		x_ptr->flags3 |= TR3_HIDE_TYPE;

	/* We are done. */
	return (TRUE);
}
