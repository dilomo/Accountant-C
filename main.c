/*=================================== main.c ========================================*/
/* All ights reserved - Anton Kerezov 2008
 * send comments on ankere@gmail.com
 */
#include "Acc.Main.h"
#include "Acc.StringStack.h"
#include "Acc.LinkedList.h"
#include "Acc.Sort.h"
#include "Acc.Xml.h"

/*The capital I refers to Incomes and O = Outcomes*/
/*============================== I/O ==================================*/
FILE * dbout = NULL;
FILE * dbin = NULL;
static const char *default_filename = "main.adb";

static char current_filename[CONF_MAX_LINE-CONF_LVAL_MAX];
static char exall_filename[CONF_MAX_LINE-CONF_LVAL_MAX];
static char exi_filename[CONF_MAX_LINE-CONF_LVAL_MAX];
static char exo_filename[CONF_MAX_LINE-CONF_LVAL_MAX];

static XmlWriter xml_w;
static XmlReader xml_r;

static bool save_to_bin(char *filepath, char *mode);
static bool load_to_app(char *filepath, char *mode);
static bool loadXML_to_app(char * filename);
static bool merge_db(FILE * mergefile);

/*save*/
static void SaveItem(recdata rp);
/*export - txt*/
static bool save_to_txt(char *filepath, char *mode, rectype t, void (*func)(recdata r));
static void SaveTxtItem(recdata rp);
static void SaveFilteredTxtItem(recdata rp);
/*export - xml*/
static bool save_to_XML(char *filepath, rectype t, void (*func)(recdata r));
static void SaveXMLItem(recdata rp);
static void SaveFilteredXMLItem(recdata rp);

/*============================== VARIABLES ==================================*/
const char ver[] = " Anton Kerezov - All rights reserved\n Version 0.4a, build 28208\n";
const char shortver[] = "0.4a";

/*list properties*/
static int   idwidth = 3;
static int   rowsperscroll = 23;
static int   max_exportwidth = 66; //from 45 to 300
static bool  add_typestats_exp = true;
static int   precision = 2;
static char  date_separator = '-';
static char  filter_expression[CONF_MAX_LINE-CONF_LVAL_MAX];
static bool  set_new_ID_onstartup = false;
static bool  rem_listpos = false;
static bool  rem_filt_listpos = false;
static int   pageI = 1, pageO = 1;
static int   pageIf = 1, pageOf = 1;
static bool  list_page_changed = false;
static bool  import_ID = false;

/*misc vars*/
static char empty[3];//to use with clear()
/*show info global vars*/
static bool hasmsg = false;
static char infomsg[INFO_LEN];

static bool date_iso_format = true;

static bool haschanges = false; //set to true if any rec added, deleted, changed smth
static bool validfilter = false;
static int  currentrow = 0;

/*used to store filtered sums*/
static long double IncomeSumF ;
static long double OutcomeSumF ;
static bool fbalance_calculated = false;

/*statistics*/
static double monthly_balance_income[13] = {
    0
}
; //starting from 1..12
static double monthly_balance_outcome[13] = {
    0
}
; //starting from 1..12
static char month_name[13][4] = { "N/A", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
                                };
static int monthly_balance_year = 1900;
static bool income_dia = true;
static bool income_types_dia = true;

CList mTypesIncomes;
CList mTypesOutcomes;
CList mTypesIncomesFiltered;
CList mTypesOutcomesFiltered;

/*============================== MISC FUNCTIONS ==================================*/
static void getvalue(double *v);
static void getdate(date *dt);
//static void readline(char **s);
static void readline_th(char **s, rectype type); /* (th = typehelp) to be used with '?' autocompletion help*/
static void readdescr(char **s);
static bool yesno(const char * fmt, ...);

static void clear(void);        /*used to get any remaining chars after menu pick*/
static void clrscr();
void printi(const char * fmt, ...);//prepares data to be printed on next menu redraw
static void showinfo(); //shows the data prepared by printi()
static void calcidwidth(rectype t);// used in show list
static void calcidwidth_rec(REC *r);
char *rval(char *s);// used in configuration parsing
static void checksave(void);    /*checks for changes and ask to sav if any*/
static char * readvalue(int maxlen, char *prompt);
static void fprintheader(FILE *out, char *header, bool print_datetime);
/*statistics*/
static void print_balance(rectype t);
static void print_filtered_balance(rectype t);
static void CalcItemIncome(recdata r);
static void CalcItemOutcome(recdata r);
static void getmonthly_balance(void);
static void AddtoMonthlyBalanceI(recdata r);
static void AddtoMonthlyBalanceO(recdata r);
static void Print_MonthlyHistogram(rectype t);
static void Print_TypesHistogram(rectype t);
static void CalculateHistogram(FILE *out, rectype t);
static void CalculateTypeHistogram(FILE *out, rectype t);
static void fprintcolumn(FILE *out, const char *s, int start, int width);
static void printcolumn(const char *s, int start, int width);
static bool get_typestats();
static void printStat(CItem item);
static void fprintStat(CItem item);

/*============================== CONFIG MANAGMENT ==================================*/
enum conftype {
    none = 0, lines_per_page, date_separator_char, value_precision, ex_all, ex_in, ex_out, date_format, current_db,
    filter_ex, set_ID_onstartup, sl_remember_last_shown, sfl_remember_last_shown, export_max_width,
    add_export_typestats, start_group, end_group, import_withID
};
static bool readconf(void);
/*WARNING: inside free() is used because of my strdupl()*/
static bool updateconf(char *lval, char *rval);
static enum conftype findconf(char *s);
static char *getconfstr(enum conftype t);

/*=================================== FILTERS ======================================*/
static Index count_filteredI = 0;
static Index count_filteredO = 0;

static struct filter_element filters[FILTER_COUNT];

static enum filtertype getftype(char *s);
static enum equation geteqtype(enum filtertype ft, char *s);
static void setfilterval(int i, char *type);
static char filterlog[6][150];
static void printfilterlog(char *newexp);
static int  filterdatacmp(struct filter_element a, struct filter_element b);
static bool compswap(struct filter_element a, struct filter_element b);
static void editfilter(void);
static void clearfilter();
static bool CollectTypeStats_filtered(rectype t);
static bool AddToTypeStatsI(recdata rp);
static bool AddToTypeStatsO(recdata rp);

/*=================================== SORTING =====================================*/
unsigned long sort_arrI_num = 0;
unsigned long sort_arrO_num = 0;

unsigned long sort_arrI_index = 0;
unsigned long sort_arrO_index = 0;

SortArray sort_arrI = NULL;
SortArray sort_arrO = NULL;

static bool fill_sortarray(rectype t);
static bool AddtoSortArrayI(REC* r);
static bool AddtoSortArrayO(REC* r);
static void find_idwidth(SortElement sl);

SortDirection mSortOrder;
SortType      mSortType;

/*=================================== MISC ========================================*/
static void PrintHelp(void);
static int drawhelp(void);

/*=================================== Program ========================================*/
int mainmenu_Menu(void) {
    int res = 0;
    clrscr();
    showinfo();

    textbox("Main Menu", TB_WIDTH);
    printf("\n 1. Add record(s) ...\n");
    printf(" 2. Show list ...\n");
    printf(" 3. Show filtered list ...\n");
    printf(" 4. Edit or delete records ...\n\n");

    printf(" 5. Statistics ...\n\n");

    printf(" 6. Save database ...\n");
    printf(" 7. Create, open and maintain ...\n");
    printf(" 8. Help ...\n");
    printf(" 9. About\n\n");
    printf(" 0. Exit\n\n");

    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > 10);

    clear();
    return res;
}

void freeAll(void) {
    clearlist(income);
    clearlist(outcome);

    EmptyTheList(&mTypesIncomes);
    EmptyTheList(&mTypesOutcomes);
    EmptyTheList(&mTypesIncomesFiltered);
    EmptyTheList(&mTypesOutcomesFiltered);

    DisposeSortArray(sort_arrI);
    DisposeSortArray(sort_arrO);
}

int main(int argc, char *argv[]) {
//    XmlWriter rd;
//
//    if (!InitXmlWriter(&rd, "text.xml"))
//        printi("failed to load xml");
//
//    SetXmlWriterFormatting(&rd ,' ', 4);
//
//    WriteStartDoc(&rd);
//    WriteStartElem(&rd, "record");
//        WriteAttributeString(&rd, "ID", "5465");
//        WriteStartElem(&rd, "hahaha");
//        WriteElementString(&rd, "name", "Jhon");
//        WriteElementString(&rd, "sname", "Osborne");
//        WriteElementString(&rd, "address", "The moony sshit");
//        WriteStartElem(&rd, "misc");
//        WriteAttributeString(&rd, "ID", "5466");
//        WriteValueString(&rd, "HOW DO YOu do");
//        WriteEndElem(&rd);
//        WriteStartElem(&rd, "haha2");
//            WriteCommentString(&rd, "Here we add shitty nothing");
//            WriteElementString(&rd, "namem", "Allise");
//            WriteCommentString(&rd, "Here we add shitty nothing");
//            WriteElementString(&rd, "city", "NY, USA");
//
//    WriteEndDoc(&rd);
//    CloseXmlWriter(&rd);

    XmlReader xr;

    if (!InitXmlReader(&xr, "text.xml"))
        printi(" 2 failed to load xml");

    while (Read(&xr) == true) {
        if ( GetNodeType(&xr) == Element) {
            printf("'%s'-", ReadElementName(&xr));

            if ( AttributeCount(&xr) > 0 )
                printf("(%s=%s)-", GetAttribureName(&xr, 0), GetAttribureValue(&xr, 0));

            if (HasValue(&xr))
                printf("'%s'\n", ReadElementString(&xr));
            else
                printf("\n");
        }
    }
    CloseXmlReader(&xr);

    printf("Closed\n");

    bool trav_all = false;
    bool load_temp = false;
    bool argumenttest = false;
    bool fail = false;

    atexit(freeAll); //THIS IS USED TO FREE MEMORY

    /*initialize database path*/
    strcpy(current_filename, default_filename);
    /*initialize export paths*/
    strcpy(exall_filename, "export_all.txt");
    strcpy(exi_filename, "export_income.txt");
    strcpy(exo_filename, "export_outcome.txt");
    char cfb[strlen(current_filename)];
    clearstr(cfb);

    if (argc == 2) {
        if (strcmp(argv[1], "-t") == 0) {
            trav_all = true;
        } else { //open the dbfile
            load_temp = true;
        }
        argumenttest = true;
    } else if (argc == 3) {
        if (strcmp(argv[1], "-t") == 0 ) {
            trav_all = true;
            load_temp = true;
            argumenttest = true;
        }
    }

    if (trav_all == false) {
        clrscr();
        clrscr();
        printf("\n Loading application configuration ... ");
    }

    if (readconf() == false) {
        /*file does not exists, create it*/
        FILE *fp = fopen(CONF_FILE, "w");
        fclose(fp);
    }

    InitialzieList();
    InitializeListPtr(&mTypesIncomes);
    InitializeListPtr(&mTypesOutcomes);
    InitializeListPtr(&mTypesIncomesFiltered);
    InitializeListPtr(&mTypesOutcomesFiltered);

    if (trav_all == false) {
        /*load db*/
        printf("OK\n Loading database '%s', please wait ... ", current_filename);
        fflush(stdout);
    }
    if (load_temp == true) {
        strcpy(cfb, current_filename);
        strcpy(current_filename, argc == 2 ? argv[1]:argv[2]);
    }

reload:
    if (load_to_app(current_filename, "rb")) {
        if (fail)
            printi(" Failed to load '%s'.\n %lu records loaded from '%s'\n", argv[1], IncomesCount + OutcomesCount, current_filename);
        else
            printi(" %lu records loaded from '%s'\n", IncomesCount + OutcomesCount, current_filename);
    } else {
        if (argumenttest) {
            /*restore original file name loaded by configuration*/
            clearstr(current_filename);
            strcpy(current_filename, cfb);
            /*try to reload old db*/
            printf(" Reloading database '%s', please wait ... \n", current_filename);
            argumenttest = false;
            fail = true;
            goto reload;
        } else
            printi(" Could not load database '%s'!\n", current_filename);
    }

    if (trav_all == true) {

        printf("\nINCOMES: \n\n");
        traverse(PrintItem, income);
        printf("\nOUTCOMES: \n\n");
        traverse(PrintItem, outcome);

        EmptyTheList(&mTypesIncomes);
        EmptyTheList(&mTypesOutcomes);

        DisposeSortArray(sort_arrI);
        DisposeSortArray(sort_arrO);

        return 0;
    }

    count_filteredI = count_filtered(income, passfilter);
    count_filteredO = count_filtered(outcome, passfilter);
    CollectTypeStats_filtered(both);

    int menuresult = 0;
    while ((menuresult = mainmenu_Menu()) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1:
            addrecord_Picker();
            break;
        case 2:
            showlist_Picker();
            break;
        case 3:
            filterrecord_Picker();
            break;
        case 4:
            Edit_Del_Records_Picker();
            break;
        case 5://histogram
            Statistics_Picker();
            break;
        case 6:
            savedb_Picker();
            break;
        case 7:
            manage_Picker();
            break;
        case 8://help
            PrintHelp();
            break;
        case 9:
            printi(ver);
            break;
        case 10:
            printi(" I devote this program to my family and\n all my friends ;)\n");
            break;
        default:
            printi(" Invalid menu number entered - %d\n", menuresult);
            break;
        }
    }

    updateconf(getconfstr(filter_ex), filter_expression);
    checksave();

    printf("\n Bye !\n\n");
    return 0;
}

int addrecord_Menu(void) {
    int res = 0;

    clrscr();
    showinfo();

    textbox("Chose Record Type", TB_WIDTH);
    printf("\n 1. Start income add sequence\n");
    printf(" 2. Start outcome add sequence\n\n");
    printf(" 3. Start typed income add sequence\n");
    printf(" 4. Start typed outcome add sequence\n\n");
    printf(" 0. Back\n\n");

    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > 4);

    clear();

    return res;
}

void addrecord_Picker(void) {
    int menuresult = 0;
    while ((menuresult = addrecord_Menu()) != 0) {

        switch (menuresult) {
        case EOF:
            printf("\n");
            break;
        case 0:
            break; //go to previous loop - main menu
        case 1:
            AddRecord(income);
            break;
        case 2:
            AddRecord(outcome);
            break;
        case 3:
            AddTypedRecord(income);
            break;
        case 4:
            AddTypedRecord(outcome);
            break;
        default:
            printi(" Invalid menu number entered - %d\n", menuresult);
            break;
        }
    }
}

void AddRecord(rectype t) {
    clrscr();
    printf("%s\n", t == income ? " ~ Incomes ~" : " ~ Outcomes ~");
    printf("Enter the neccessary information:\n\n");
    int c = count(t);

    do {
        recdata newrec;

        getvalue(&newrec.value);
        readline_th(&newrec.type, t);
        readdescr(&newrec.descr);
        getdate(&newrec.date);

        if (addtolist(newrec, t) == false) {
            printe("Could not add record to DB!");
            printi("Could not add record to DB!");
            return;
        }

        CItem * pitm;
        CItem titm;
        strcpy(titm.text, newrec.type);
        titm.total = newrec.value;
        titm.num = 1;
        if ((pitm = FindItem(titm, t == income? &mTypesIncomes: &mTypesOutcomes, cmpCItems)) == NULL)
            AddItem(titm, t == income? &mTypesIncomes: &mTypesOutcomes);
        else {
            pitm->total += newrec.value;
            pitm->num += 1;
        }

        fflush(stdin);
    } while (yesno("Add new [Y/n]:"));

    if (c < count(t))
        haschanges = true;
}

void AddTypedRecord(rectype t) {
    clrscr();
//    if ( ListItemCount( t == income ? &mTypesIncomes : &mTypesOutcomes) <= 0 ) {
//        printi(
//        " There are no types available. Please submit at least one\n record of any type and then use that feature.\n");
//        return;
//    }
    char *rec_type;

showagain: // if you enter empty string

    rec_type = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Enter the type of the records you wish to submit, 0 to go back: ");
    if (rec_type == NULL) {
        free(rec_type);
        return;
    }
    CItem itm;
    strcpy(itm.text, rec_type);

    /*if not present show available types and go back*/
    if (!ContainsItem(itm, t == income ? &mTypesIncomes : &mTypesOutcomes, cmpCItems)) {
        clrscr();
        printf(" The type '%s' is not present.\n",rec_type);
        if (!yesno(" Do you want to start adding records with this type anyway?\n Press 0 to see available types. [Y/n]: ")) {
            clrscr();

            printf("Available types are:\n");
            switch (t) {
            case income:
                Traverse(&mTypesIncomes, printCItem);
                break;
            case outcome:
                Traverse(&mTypesOutcomes, printCItem);
                break;
            case both:
                printf("Income types:\n");
                Traverse(&mTypesIncomes, printCItem);
                printf("\nOutcome types:\n");
                Traverse(&mTypesOutcomes, printCItem);
                break;
            }
            printf("\n");
            free(rec_type);
            goto showagain;
        }
    }

    clrscr();
    printf("%s for type [%s] ~ \n", t == income ? " ~ Incomes" : " ~ Outcomes", rec_type);
    printf("Enter the neccessary information:\n\n");
    int c = count(t);

    do {
        recdata newrec;

        getvalue(&newrec.value);
        newrec.type = strdup_fixed(rec_type,CONF_MAX_LINE - CONF_LVAL_MAX);
        readdescr(&newrec.descr);
        getdate(&newrec.date);

        if (addtolist(newrec, t) == false)
            printe("Could not add record to DB!");

        CItem * pitm;
        CItem titm;
        strcpy(titm.text, newrec.type);
        titm.total = newrec.value;
        titm.num = 1;
        if ((pitm = FindItem(titm,t == income? &mTypesIncomes: &mTypesOutcomes, cmpCItems)) == NULL)
            AddItem(titm, t == income? &mTypesIncomes: &mTypesOutcomes);
        else {
            pitm->total += newrec.value;
            pitm->num += 1;
        }

        fflush(stdin);
    } while (yesno("Add new [Y/n]:"));

    free(rec_type);
    if (c < count(t))
        haschanges = true;
}

int filterrecord_Menu(void) {
    int res = 0;

    clrscr();
    showinfo();

    textbox("Manage Records", TB_WIDTH);
    printf("\n 1. Show Filtered List ...\n\n");

    printf(" 2. Edit Filter ...\n");
    printf(" 3. Clear Filter\n\n");
    printf(" 0. Back\n\n");

    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > 4);

    clear();

    return res;
}

void filterrecord_Picker(void) {
    int menuresult = 0;

    while ((menuresult = filterrecord_Menu()) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1:
            showfilteredlist_Picker();
            break;
        case 2:
            editfilter();
            break;
        case 3:
            clearfilter();
            break;
        default:
            printi(" Invalid menu number entered - %d\n", menuresult);
            break;
        }
    }
}

int Edit_Del_Records_Menu(void) {
    int res = 0;

    clrscr();
    showinfo();

    textbox("Edit or Delete Records of Type", TB_WIDTH);
    printf("\n 1. Income records ...\n");
    printf(" 2. Outcome records ...\n\n");

    printf(" 0. Back\n\n");

    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > 2);

    clear();
    return res;
}

void Edit_Del_Records_Picker(void) {
    int menuresult = 0;
    while ((menuresult = Edit_Del_Records_Menu()) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1:
            Edit_Del_RecordsSeq_Picker(income);
            break;
        case 2:
            Edit_Del_RecordsSeq_Picker(outcome);
            break;
        default:
            printi(" Invalid menu number entered - %d\n", menuresult);
            break;
        }
    }

}

int Edit_Del_RecordsSeq_Menu(char *s, rectype t) {
    int res = 0;
    int menues = 2;

    clrscr();
    showinfo();

    textbox(s, TB_WIDTH);
    if (listisempty(t)) {
        printf("\n\n  <no records added>\n\n");
        menues = 0;
    } else {
        printf("\n 1. Start edit sequence ... \n");
        printf(" 2. Start delete sequence ...\n\n");
        menues = 2;
    }
    printf(" 0. Back\n\n");
    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > menues);

    clear();
    return res;
}

void Edit_Del_RecordsSeq_Picker(rectype t) {
    int menuresult = 0;

    while ((menuresult = Edit_Del_RecordsSeq_Menu(t == income ? "Incomes Managment" : "Outcomes Managment", t)) != 0) {
        switch (menuresult) {
        case 0 :
            break;
        case 1:
            editrecord_seq(t);
            break;
        case 2:
            deleterecord_seq(t);
            break;
        default:
            printi(" Invalid menu number entered - %d\n", menuresult);
            break;
        }
    }
}

int showlist_Menu(void) {
    int res = 0;

    clrscr();
    showinfo();

    textbox("Show List - Record type", TB_WIDTH);
    printf("\n 1. Income records - %lu items\n", IncomesCount);
    printf(" 2. Outcome records - %lu items\n\n", OutcomesCount);

    printf(" 0. Back\n\n");

    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > 2);

    clear();
    return res;
}

void showlist_Picker(void) {
    int menuresult = 0;
    while ((menuresult = showlist_Menu()) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1:
            /*this check is safety guard if no records are added. DO NOT REMOVE*/
            if (IncomesCount == 0) {
                printi(" No records to list.\n");
                break;
            }

            ShowList(PrintItem, income);

            break;
        case 2:
            /*this check is safety guard if no records are added. DO NOT REMOVE*/
            if (OutcomesCount == 0) {
                printi(" No records to list.\n");
                break;
            }

            ShowList(PrintItem, outcome);

            break;
        default:
            printi(" Invalid menu number entered - %d\n", menuresult);
            break;
        }
    }
}

int showfilteredlist_Menu(void) {
    int res = 0;

    clrscr();
    showinfo();

    textbox("Show Filtered List - Record type", TB_WIDTH);
    printf("\n 1. Income records - %lu items ...\n", count_filteredI);
    printf(" 2. Outcome records - %lu items ...\n\n", count_filteredO);

    printf(" 0. Back\n\n");

    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > 2);

    clear();
    return res;
}

void showfilteredlist_Picker(void) {
    int menuresult = 0;
    while ((menuresult = showfilteredlist_Menu()) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1:
            check_show_filter(income);
            break;
        case 2:
            check_show_filter(outcome);
            break;
        default:
            printi(" Invalid menu number entered - %d\n", menuresult);
            break;
        }
    }

}

int export_Menu(void) {
    int res = 0;
    clrscr();
    showinfo();
    int items = 3;

    textbox("Export Options - Text File", TB_WIDTH);
    printf("\n 1. Save all\n");
    printf(" 2. Save income only\n");
    printf(" 3. Save outcome only\n\n");

    if (validfilter && strlen(filter_expression) > 0) {
        printf(" 4. Save all (using filters)\n");
        printf(" 5. Save income only (using filters)\n");
        printf(" 6. Save outcome only (using filters)\n\n");
        items = 6;
    }
    printf(" 0. Back\n\n");

    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > items);

    clear();

    return res;
}

void export_Picker(void) {
    int menuresult = 0;

    while ((menuresult = export_Menu()) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1:
            clrscr();
            printf("Saveing data, please wait ...\n");

            save_to_txt(exall_filename, "w", both, SaveTxtItem);

            printi(" Data saved to '%s'\n", exall_filename);
            break;
        case 2:
            clrscr();
            printf("Saveing data, please wait ...\n");

            save_to_txt(exi_filename, "w", income, SaveTxtItem);

            printi(" Data saved to '%s'\n", exi_filename);
            break;
        case 3:
            clrscr();
            printf("Saveing data, please wait ...\n");

            save_to_txt(exo_filename, "w", outcome, SaveTxtItem);

            printi(" Data saved to '%s'\n", exo_filename);
            break;
        case 4:
            clrscr();
            printf("Saveing data, please wait ...\n");

            save_to_txt(exall_filename, "w", both, SaveFilteredTxtItem);

            printi(" Data saved to '%s'\n", exall_filename);
            break;
        case 5:
            clrscr();
            printf("Saveing data, please wait ...\n");

            save_to_txt(exi_filename, "w", income, SaveFilteredTxtItem);

            printi(" Data saved to '%s'\n", exi_filename);
            break;
        case 6:
            clrscr();
            printf("Saveing data, please wait ...\n");

            save_to_txt(exo_filename, "w", outcome, SaveFilteredTxtItem);

            printi(" Data saved to '%s'\n", exo_filename);
            break;
        default:
            printi(" Invalid menu number entered - %d\n", menuresult);
            break;
        }
    }
}

int exportXML_Menu(void) {
    int res = 0;
    clrscr();
    showinfo();
    int items = 3;

    textbox("Export Options - XML File", TB_WIDTH);
    printf("\n 1. Save all\n");
    printf(" 2. Save income only\n");
    printf(" 3. Save outcome only\n\n");

    if (validfilter && strlen(filter_expression) > 0) {
        printf(" 4. Save all (using filters)\n");
        printf(" 5. Save income only (using filters)\n");
        printf(" 6. Save outcome only (using filters)\n\n");
        items = 6;
    }
    printf(" 0. Back\n\n");

    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > items);

    clear();

    return res;
}

void exportXML_Picker(void) {
    int menuresult = 0;
    char * fname = NULL;

    while ((menuresult = exportXML_Menu()) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1:
            clrscr();
            fname = readvalue(MAX_LINE, "Enter the name of the file to export to: ");
            if (fname == NULL) {
                free(fname);
                break;
            }
            trim('"', fname);
            /*add extension*/
            if (!strstr(fname, ".xml"))
                strcat(fname, ".xml");


            printf("Saveing data, please wait ...\n");
            if (save_to_XML(fname, both, SaveXMLItem))
                printi(" Data saved to '%s'\n", fname);
            else
                printi(" Could not export data to '%s'\n", fname);
            break;
        case 2:
            clrscr();
            fname = readvalue(MAX_LINE, "Enter the name of the file to export to: ");
            if (fname == NULL) {
                free(fname);
                break;
            }
            trim('"', fname);
            /*add extension*/
            if (!strstr(fname, ".xml"))
                strcat(fname, ".xml");

            printf("Saveing data, please wait ...\n");
            if (save_to_XML(fname, income, SaveXMLItem))
                printi(" Data saved to '%s'\n", fname);
            else
                printi(" Could not export data to '%s'\n", fname);
            break;
        case 3:
            clrscr();
            fname = readvalue(MAX_LINE, "Enter the name of the file to export to: ");
            if (fname == NULL) {
                free(fname);
                break;
            }
            trim('"', fname);
            /*add extension*/
            if (!strstr(fname, ".xml"))
                strcat(fname, ".xml");

            printf("Saveing data, please wait ...\n");
            if (save_to_XML(fname, outcome, SaveXMLItem))
                printi(" Data saved to '%s'\n", fname);
            else
                printi(" Could not export data to '%s'\n", fname);
            break;
        case 4:
            clrscr();
            fname = readvalue(MAX_LINE, "Enter the name of the file to export to: ");
            if (fname == NULL) {
                free(fname);
                break;
            }
            trim('"', fname);
            /*add extension*/
            if (!strstr(fname, ".xml"))
                strcat(fname, ".xml");

            printf("Saveing data, please wait ...\n");
            if (save_to_XML(fname, both, SaveFilteredXMLItem))
                printi(" Data saved to '%s'\n", fname);
            else
                printi(" Could not export data to '%s'\n", fname);
            break;
        case 5:
            clrscr();
            fname = readvalue(MAX_LINE, "Enter the name of the file to export to: ");
            if (fname == NULL) {
                free(fname);
                break;
            }
            trim('"', fname);
            /*add extension*/
            if (!strstr(fname, ".xml"))
                strcat(fname, ".xml");

            printf("Saveing data, please wait ...\n");
            if (save_to_XML(fname, income, SaveFilteredXMLItem))
                printi(" Data saved to '%s'\n", fname);
            else
                printi(" Could not export data to '%s'\n", fname);
            break;
        case 6:
            clrscr();
            fname = readvalue(MAX_LINE, "Enter the name of the file to export to: ");
            if (fname == NULL) {
                free(fname);
                break;
            }
            trim('"', fname);
            /*add extension*/
            if (!strstr(fname, ".xml"))
                strcat(fname, ".xml");

            printf("Saveing data, please wait ...\n");
            if (save_to_XML(fname, outcome, SaveFilteredXMLItem))
                printi(" Data saved to '%s'\n", fname);
            else
                printi(" Could not export data to '%s'\n", fname);
            break;
        default:
            printi(" Invalid menu number entered - %d\n", menuresult);
            break;
        }
    }
    free(fname);
}


int savedb_Menu(void) {
    int res = 0;
    clrscr();
    showinfo();

    textbox("Save Database", TB_WIDTH);
    printf("\n 1. Update current\n\n");
    printf(" 2. Save current as ...\n");
    printf(" 3. Save a copy ...\n\n");

    printf(" 4. Export to text file ...\n");
    printf(" 5. Export to XML file ...\n\n");

    printf(" 0. Back\n\n");

    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > 5);

    clear();

    return res;
}

void savedb_Picker(void) {
    int menuresult = 0;
    // int c = 0;

    while ((menuresult = savedb_Menu()) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1://Update DB
            clrscr();
            printf("Saveing data, please wait ...\n");

            save_to_bin(current_filename, "wb");

            printi(" Database updated successfully.\n DB file: '%s'\n", current_filename);
            break;
        case 2:// Save as ..
            if (saveAsDatabase()) {
                printi(" Database successfully saved to '%s'\n", current_filename);
            } else
                printi(" Could not save database. Check the access mode of acc.conf file.\n It should be rw- (allowed for reading and writing)\n");

            break;
        case 3:// Make a copy
            if (makeCopyDatabase())
                printi(" Database copy saved.\n Now using: '%s'\n", current_filename);
            else
                printi(" Could not copy database. Check the access mode of 'acc.conf' file.\n It should be rw- (allowed for reading and writing)\n");

            break;
        case 4:
            export_Picker();
            break;
        case 5:
            exportXML_Picker();
            break;
        default:
            printi(" Invalid menu number entered - %d\n", menuresult);
            break;
        }
    }
}

bool createDatabase(void) {
    bool result = false;

    checksave();
    clrscr();
    char * new = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Type the name of the new database: ");
    if (new == NULL) {
        free(new);
        return true;
    }
    trim('"', new);

    /*check and add extention if necessary*/
    if (!strstr(new, ".adb"))
        strcat(new, ".adb");

    clrscr();
    printf("\n Unloading database '%s', please wait ...\n", current_filename);
    fflush(stdout);

    freeAll();
    InitialzieList();

    save_to_bin(new, "wb");

    if ((result = updateconf(getconfstr(current_db), new))) {
        clearstr(current_filename);
        strcpy(current_filename, new);
    }

    free(new);

    return result;
}

static bool loadXML_to_app(char * filename) {
    if (!InitXmlReader(&xml_r, filename)) {
        printe("Could not open Xml file '%s'", filename);
        return false;
    } else
        printf(" Parsing xml document ...\n");

    rectype tmp_t = income;
    bool start_rec = false;
    recdata tmp_rec;
    /* TODO (toni#1#): Update typestats
    */


    while (Read(&xml_r)) {
        switch (GetNodeType(&xml_r)) {
        case Element:

            /*get rec info*/
            if (start_rec) {
                /* get date */
                if (strcmp(ReadElementName(&xml_r), XML_DATE_NAME) == 0 && HasValue(&xml_r)) {
                    if (!parsedate(ReadElementString(&xml_r), &tmp_rec.date)) {
                        printe(" Sub Element %s of record with ID=%s has invalid date", ReadElementName(&xml_r), tmp_rec.ID);
                        CloseXmlReader(&xml_r);
                        return false;
                    }
                    break;
                } else {
                    printe(" Sub Element %s of record with ID=%s has no value", ReadElementName(&xml_r), tmp_rec.ID);
                    CloseXmlReader(&xml_r);
                    return false;
                }
                /* value */
                if (strcmp(ReadElementName(&xml_r), XML_VALUE_NAME) == 0 && HasValue(&xml_r)) {
                    tmp_rec.value = atofp(ReadElementString(&xml_r), precision);
                    break;
                } else {
                    printe(" Sub Element %s of record with ID=%s has no value", ReadElementName(&xml_r), tmp_rec.ID);
                    CloseXmlReader(&xml_r);
                    return false;
                }

                /* type */
                if (strcmp(ReadElementName(&xml_r), XML_TYPE_NAME) == 0 && HasValue(&xml_r)) {
                    tmp_rec.type = strdupl(ReadElementString(&xml_r));
                    break;
                } else {
                    printe(" Sub Element %s of record with ID=%s has no value", ReadElementName(&xml_r), tmp_rec.ID);
                    CloseXmlReader(&xml_r);
                    return false;
                }
                /* description */
                if (strcmp(ReadElementName(&xml_r), XML_DESCR_NAME) == 0 && HasValue(&xml_r)) {
                    tmp_rec.type = strdupl(ReadElementString(&xml_r));
                    break;
                } else {
                    tmp_rec.type = NULL;
                }
            }

            /*start record element*/
            if (strcmp(ReadElementName(&xml_r), XML_REC_NAME) == 0 && HasSubNodes(&xml_r)) {
                start_rec = true;
                tmp_rec.ID = atoul(GetAttribureValue(&xml_r, 0));
                break;
            }

            /*check if is income type yet*/
            if ( strcmp(ReadElementName(&xml_r), "Outcomes") == 0){
                tmp_t = outcome;
                break;
            }

            /*root node - cmp app version of file and program*/
            if ( strcmp(ReadElementName(&xml_r), XML_ROOT_NAME) == 0) {
                if (strcmp(GetAttribureName(&xml_r, 0), XML_APP_VER) == 0)
                    if (strcmp(GetAttribureValue(&xml_r, 0),(char *) shortver) != 0) {
                        printe("The file version is not the same as the app version");
                        CloseXmlReader(&xml_r);
                        return false;
                    }
            }
            break;
        case EndElement:
            if (import_ID == true){
                if (!addtolist_withID(tmp_rec, tmp_t)){
                    printe("Could not add record with ID=%ul", tmp_rec.ID);
                    CloseXmlReader(&xml_r);
                    return false;
                }
            } else {
                if (!addtolist(tmp_rec, tmp_t)){
                    printe("Could not add record with ID=%ul", tmp_rec.ID);
                    CloseXmlReader(&xml_r);
                    return false;
                }
            }
            start_rec = false;
            break;
        case Document:
            /*skip as it has no info to use from here*/
            break;
        case Comment:
            printf("comment: %s\n", ReadElementString(&xml_r));
            break;
        case None:
            break;
        }
    }
    printf("/n Parsing OK. Cleaning up ...");
    return (CloseXmlReader(&xml_r));
}

int manage_Menu(void) {
    int res = 0;
    clrscr();
    showinfo();

    textbox("Manage databases", TB_WIDTH);
    printf("\n 1. Create new ...\n");
    printf(" 2. Open existing ...\n");
    printf(" 3. Import from XML ...\n\n");

    printf(" 4. Reset ID values ...\n");
    printf(" 5. Merge with database ...\n\n");
    printf(" 0. Back\n\n");

    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > 5);

    clear();

    return res;
}


void manage_Picker(void) {
    int menuresult = 0;
    char *new;
//    int c = 0;

    while ((menuresult = manage_Menu()) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1: //Create
            if (createDatabase()) {
                printi(" Now using '%s'\n", current_filename);
            } else
                printi(" Could not save database. Check the access mode of acc.conf file.\n It should be rw- (allowed for reading and writing)\n");

            break;
        case 2://Open
            /* @todo (A. Kerezov#5#): Restructure code preperly */
            checksave();

            clrscr();
            new = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Type the name of the database to open: ");
            if (new == NULL) {
                free(new);
                break;
            }
            trim('"', new);

            clrscr();
            printf("\n Unloading database '%s', please wait ...", current_filename);
            fflush(stdout);
            freeAll();

            InitialzieList();

            printf("\n Loading database '%s', please wait ...\n", new);
            fflush(stdout);

            if (load_to_app(new, "rb")) {
                updateconf(getconfstr(current_db), new);
                strncpy(current_filename, new, CONF_MAX_LINE - CONF_LVAL_MAX);
                printi(" %lu records loaded from %s\n", IncomesCount + OutcomesCount, new);
            } else
                printi(" Could not load database '%s'!\n", new);

            free(new);
            break;
        case 4://Refresh IDs
            clrscr();
            if (!yesno("Do you want to give all the records in the database '%s'\nnew ID numbers?[Y/n]: ", current_filename))
                break;
            clrscr();
            printf("Refreshing IDs, please wait ...\n");

            refreshIndex(income);
            refreshIndex(outcome);
            haschanges = true;

            printi(" ID values successfully updated.\n");

            break;
        case 5://Copy to
            clrscr();
            new = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Type the name of the database to merge with: ");
            if (new == NULL) {
                free(new);
                break;
            }
            trim('"', new);
            FILE * merge = fopen(new, "rb");
            if (merge == NULL) {
                printi(" Could not open file: %s\n", new);
                break;
            }
            clrscr();
            if (!yesno("Do you want to merge the two databases into one?\nCopy to  : '%s'\nCopy from: '%s'\n[Y/n]: ",
                       current_filename, new))
                break;

            clrscr();
            printf("Merge in action, please wait ...\n");
            if (merge_db(merge)) {
                count_filteredI = count_filtered(income, passfilter);
                count_filteredO = count_filtered(outcome, passfilter);
                printi(" Merge successful.\n Loaded '%s'\n", new);
            } else
                printi(" Merge failed! Probably the file '%s'\n could not be opened.\n", new);

            haschanges = true;
            free(new);
            break;
        case 3: /*Import XML */
            clrscr();
            new = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Type the name of the file to import: ");
            if (new == NULL) {
                free(new);
                break;
            }
            trim('"', new);
            if (loadXML_to_app(new)) {
                printi(" Import Successful. '%ul' records loaded.\n", IncomesCount+OutcomesCount);
            } else
                printi(" Could not import data. Check if the filename\n is correctly spelled. Check the file for errors.");

            free(new);
            break;
        default:
            printi(" Invalid menu number entered - %d\n", menuresult);
            break;
        }
    }
}

int Statistics_Menu(void) {
    int res = 0;
    clrscr();
    showinfo();

    textbox("Statistics", TB_WIDTH);
    printf("\n 1. Calculate balance\n\n");
    printf(" 2. Calculate filtered balance\n");
    printf(" 3. Edit filter\n");
    printf(" 4. Clear filter\n\n");

    printf(" 5. Show monthly statistics\n\n");
//    printf(" 6. Show statistics by type\n\n");
//    printf(" 6. Show monthly histogram\n");
//    printf(" 7. Show annual histogram\n\n");
    printf(" 0. Back\n\n");
    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > 6);

    clear();

    return res;
}

void Statistics_Picker(void) {
    int menuresult = 0;

    while ((menuresult = Statistics_Menu()) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1:
            Balance_Picker(both);
            break;
        case 2:
            FilteredBalance_Picker(both);
            break;
        case 3:
            editfilter();
            break;
        case 4:
            clearfilter();
            break;
        case 5:
            /* @todo (A. Kerezov#4#): Make the body of the
            break statement part
            of a function */
            //get the year to display
            ;
            char *new;
showagain:
            clrscr();
            new = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Enter the year to display diagram for, 0 to go back (1900-9999): ");
            if (new == NULL) {
                free(new);
                break;
            }

            monthly_balance_year = atoi(new);

            if ( monthly_balance_year < 1900 || monthly_balance_year > 9999) {
                free(new);
                goto showagain;
            }
            clrscr();
            printf("Gathering data ...\n");
            MonthHistogram_Picker(both);

            free(new);
            break;
        case 6:
//                clrscr();
//                printf("Gathering data ...\n");
//                TypeHistogram_Picker(both);
            break;
        case 7:
            break;
        default:
            printi(" Invalid menu number entred - %d\n", menuresult);
            break;
        }
    }
}
/*preferable way of menu draw*/
int Custom_Menu(int menuelements, void (*func)(rectype t), rectype t) {
    int res = 0;
    clrscr();

    /*CALL FUNCTION THAT IS RESPONSIBLE FOR DRAWING*/
    (*func)(t);


    showinfo();
    do {
        printf("Chose entry: ");

        scanf("%i", &res);
    } while (res < 0 || res > menuelements);

    clear();
    return res;
}

void Balance_Picker(rectype t) {
    int menuresult = 0;

    while ((menuresult = Custom_Menu(1, print_balance, t)) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1:// save to file
            /* @todo (A. Kerezov#4#): Restructure code
            Try to avoid duplications */

            clrscr();
            char *new = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Enter name for the file: ");
            if (new == NULL) {
                free(new);
                break;
            }
            trim('"', new);
            FILE *fout = fopen(new, "w");
            if (fout != NULL) {
                fprintheader(fout, "Balance Report", true);
                fprintf(fout, " %20.*Lf (incomes)\n", precision, IncomeSum);
                fprintf(fout, "-%20.*Lf (outcomes)\n", precision, OutcomeSum);
                fprintln(fout, '-', TB_WIDTH);
                fprintf(fout, "=%20.*Lf (balance)\n\n", precision, IncomeSum - OutcomeSum);

                if (fclose(fout) != 0)
                    printi("Could not save data to '%s'.\n", new);
                else
                    printi("File saved to '%s'\n", new);
            } else
                printi("Could not save data to '%s'.\n", new);

            free(new);
            break;
        default:
            printi(" Invalid menu number entred - %d\n", menuresult);
            break;
        }
    }
}

static void print_balance(rectype t) {
    printf(" %20.*Lf (incomes)\n", precision, IncomeSum);
    printf("-%20.*Lf (outcomes)\n", precision, OutcomeSum);
    println('-', TB_WIDTH);
    printf("=%20.*Lf (balance)\n\n", precision, IncomeSum - OutcomeSum);

    println('*', TB_WIDTH);
    printf("\n 1. Save to file ...\t0. Back\n\n");
}

static void print_filtered_balance(rectype t) {
    /* calculate the balance only when there is changes to the db
     * or at the start of the program
     */
    if (haschanges || !fbalance_calculated) {
        OutcomeSumF = 0;
        IncomeSumF = 0;
        traverse(CalcItemIncome, income);
        traverse(CalcItemOutcome, outcome);
        fbalance_calculated = true;
    }

    printf(" %20.*Lf (incomes)\n", precision, IncomeSumF);
    printf("-%20.*Lf (outcomes)\n", precision, OutcomeSumF);
    println('-', TB_WIDTH);
    printf("=%20.*Lf (balance)\n\n", precision, IncomeSumF - OutcomeSumF);

    println('*', TB_WIDTH);
    printf("\n 1. Save to file ...\t0. Back\n\n");

}

void FilteredBalance_Picker(rectype t) {
    int menuresult = 0;

    while ((menuresult = Custom_Menu(1, print_filtered_balance, t)) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1:// save to file
            /* @todo (A. Kerezov#4#): Restructure code
               Try to avoid duplications */
            clrscr();
            char *new = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Enter name for the file: ");
            if (new == NULL) {
                free(new);
                break;
            }
            trim('"', new);
            FILE *fout = fopen(new, "w");
            if (fout != NULL) {
                fprintheader(fout, "Filtered Balance Report", true);
                fprintf(fout, " %20.*Lf (incomes)\n", precision, IncomeSumF);
                fprintf(fout, "-%20.*Lf (outcomes)\n", precision, OutcomeSumF);
                fprintln(fout, '-', TB_WIDTH);
                fprintf(fout, "=%20.*Lf (balance)\n\n", precision, IncomeSumF - OutcomeSumF);

                if (fclose(fout) != 0)
                    printi("Could not save data to '%s'.\n", new);
                else
                    printi("File saved to '%s'\n", new);
            } else
                printi("Could not save data to '%s'.\n", new);

            free(new);
            break;
        default:
            printi(" Invalid menu number entred - %d\n", menuresult);
            break;
        }
    }

}

void MonthHistogram_Picker(rectype t) {

    int menuresult = 0;
    income_dia = true;
    getmonthly_balance();

    while ((menuresult = Custom_Menu(2, Print_MonthlyHistogram, t)) != 0) {
        switch (menuresult) {
        case 0:
            break;
        case 1:// save to file
            /* @todo (A. Kerezov#5#): Restructure code
               Try to avoid duplications */
            clrscr();
            char *new = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Enter name for the file: ");
            if (new == NULL) {
                free(new);
                break;
            }
            trim('"', new);
            FILE *fout = fopen(new, "w");
            if (fout != NULL) {
                fprintheader(fout, "Monthly Balance", true);
                if (income_dia) {
                    // fprintf(fout, "Incomes for the year %d\n\n", monthly_balance_year);
                    CalculateHistogram(fout, income);
                } else {
                    //  fprintf(fout, "\n\nOutcomes for the year %d\n\n", monthly_balance_year);
                    CalculateHistogram(fout, outcome);
                }


                if (fclose(fout) != 0)
                    printi("Could not save data to '%s'.\n", new);
                else
                    printi("File saved to '%s'\n", new);
            } else
                printi("Could not save data to '%s'.\n", new);

            free(new);
            break;
        case 2:
            if (income_dia)
                income_dia = false;
            else
                income_dia = true;
            break;
        default:
            printi(" Invalid menu number entred - %d\n", menuresult);
            break;
        }
    }
}

void TypeHistogram_Picker(rectype t) {
    int menuresult = 0;
    income_types_dia = true;

    get_typestats();

    while ((menuresult = Custom_Menu(3, Print_TypesHistogram, t)) != 0) {
        switch (menuresult) {
        case 0:
            //go back
            break;
        case 1:// save to file
            clrscr();
            char *new = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Enter name for the file: ");
            if (new == NULL) {
                free(new);
                break;
            }
            trim('"', new);
            FILE *fout = fopen(new, "w");
            if (fout != NULL) {
                fprintheader(fout, "Types Summary", true);

                if (income_dia) {
                    CalculateTypeHistogram(fout, income);
                } else {
                    CalculateTypeHistogram(fout, outcome);
                }

                if (fclose(fout) != 0)
                    printi("Could not save data to '%s'.\n", new);
                else
                    printi("File saved to '%s'\n", new);
            } else
                printi("Could not save data to '%s'.\n", new);

            free(new);
            break;
        case 2:
            if (income_types_dia)
                income_types_dia = false;
            else
                income_types_dia = true;
            break;
        case 3:
            //next page (if any)

            break;
        default:
            printi(" Invalid menu number entred - %d\n", menuresult);
            break;
        }
    }

}

static void Print_MonthlyHistogram(rectype t) {
    if (income_dia)
        CalculateHistogram(stdout, income);
    else
        CalculateHistogram(stdout, outcome);

    printf("\n 1. Save to file ...\t2. %s \t0. Back\n\n", (income_dia == true ? "Show Outcomes Diagram" : "Show Incomes Diagram"));
}

static void Print_TypesHistogram(rectype t) {
    if (income_types_dia)
        CalculateTypeHistogram(stdout, t);
    else
        CalculateTypeHistogram(stdout, t);

    printf("\n 1. Save to file...\t2. %s \t3. Next Page\t0. Back\n\n", (income_types_dia == true ? "Show Outcome Types" : "Show Income Types"));
}

static void CalcItemIncome(recdata r) {
    if (passfilter(r))
        IncomeSumF += r.value;
}

static void CalcItemOutcome(recdata r) {
    if (passfilter(r))
        OutcomeSumF += r.value;
}

static void CalculateHistogram(FILE *out, rectype t) {
    //one row totals 80 chars
    char tmp[SCREEN_WIDTH + 1];
    sprintf(tmp, "%s - %d", (income_dia == false ? "Outcomes Diagram" : "Incomes Diagram"), monthly_balance_year);

    fprintmid(out, tmp , SCREEN_WIDTH);
    fprintln(out, '_', SCREEN_WIDTH);
    fprintf(out, "\n");

    double *arr = (t == income ? monthly_balance_income : monthly_balance_outcome);

    int i;
    double max = 0;
    double percent = 0;

    for (i = 1 ; i < 13; i++) {
        if (arr[i] > max)
            max = arr[i];
    }
    if (max <= 0)
        max = 100;
    percent = max / 100;



    for (i = 1 ; i < 13; i++) {
        fprintf(out, "%*.*f %s ", 20 - precision, precision, arr[i], month_name[i]);

        int p = 0;
        float cp = arr[i] / percent;
        if (arr[i] == max)
            cp = 100;

        while ( p < cp ) {
            if ((p + 1) > cp)
                fprintf(out, ".");
            else
                fprintf(out, ":");

            p += 2;
        }
        fprintf(out, " %.1f%%\n", cp);
    }
    fprintln(out, '_', SCREEN_WIDTH);
}

static void CalculateTypeHistogram(FILE *out, rectype t) {
    //one row totals 80 chars
    char tmp[SCREEN_WIDTH + 1];
    sprintf(tmp, "%s", (income_types_dia == false ? "Outcomes Types Summary" : "Incomes Types Summary"));

    fprintmid(out, tmp , SCREEN_WIDTH);
    fprintln(out, '_', SCREEN_WIDTH);
    fprintf(out, "\n");
    /*print ist here*/
    switch (t) {
    case income:
        Traverse_SortArray_Default(sort_arrI, sort_arrI_num);
        break;
    case outcome:
        Traverse_SortArray_Default(sort_arrO, sort_arrO_num);
        break;
    case both:
        Traverse_SortArray_Default(sort_arrI, sort_arrI_num);
        printf("\n OUTCOMES: \n\n");
        Traverse_SortArray_Default(sort_arrO, sort_arrO_num);
        break;
    }
    fprintln(out, '_', SCREEN_WIDTH);
}


//###############################################################################################################
//###############################################################################################################
//###############################################################################################################



//###############################################################################################################
//###############################################################################################################
//###############################################################################################################


static void getvalue(double *v) {
    char str[MAX_LINE] = {' '};
    int c = 0;
    double val = -4;
    printf("Value\t:");

    do {
        while ((c = getline(str, MAX_LINE)) < 2) {
            if (str[0] != '\n')
                printf("err Value:");
            else
                printf("Value\t:");
        }

        if (onlydigits(str) == false) {
            printf("err Value:");
            continue;
        }

        val = atofp(str, precision);

        if (val < 0)
            printf("err Value:");

    } while (val < 0);

    *v = val;
}

static void getdate(date *dt) {
    date d;
    char str[MAX_LINE] = {' '};
    bool parseok = true;
    int len = 0;
    printf("Date\t:");
    do {
        if (parseok == false)
            printf("err Date:");

        while ((len = getline(str, MAX_LINE)) < 1)
            printf("err Date:");


        /*if no date is entered then use today's one*/
        if (len <= 1) {
            date_now(&d);
            break;
        }


    } while ((parseok = parsedate(str, &d)) == false);

    *dt = d;
    //printf("%i %d %d ",dt->year,dt->month,dt->day);

}

//static void readline(char **s)
//{
//    char str[MAX_LINE];
//    clearstr(str);
//
//    int len = 0;
//
//    printf("Type\t:");
//    while ((len = getline(str, MAX_LINE)) < 2)
//        printf("Type\t:"); //must be non NULL
//
//    str[len-1] = '\0';//removes \n from string
//
//    *s = strdup_fixed(str, MAX_LINE);
//}

static void readline_th(char **s, rectype type) {
    char str[MAX_LINE];
    clearstr(str);

    int len = 0;

TYPE_HELP_SHOWN:

    printf("Type\t:");
    while ((len = getline(str, MAX_LINE)) < 2)
        printf("Type\t:"); //must be non NULL

    if (len >= 2 && str[0] == '?') { //display list with available types
        switch (type) {
        case income:
            Traverse(&mTypesIncomes, printCItem);
            break;
        case outcome:
            Traverse(&mTypesOutcomes, printCItem);
            break;
        case both:
            printf("Income types:\n");
            Traverse(&mTypesIncomes, printCItem);
            printf("\nOutcome types:\n");
            Traverse(&mTypesOutcomes, printCItem);
            break;
        }
        printf("\n");
        goto TYPE_HELP_SHOWN;
    }

    str[len-1] = '\0';//removes \n from string

    *s = strdup_fixed(str, MAX_LINE);
}

static void readdescr(char **s) {
    char str[MAX_LINE];
    clearstr(str);

    int len = 0;

    printf("Descr\t:");
    while ((len = getline(str, MAX_LINE)) < 1)
        printf("Descr\t:");

    str[len-1] = '\0';//removes \n from string
    if ( len <=1 )
        *s = NULL;
    else
        *s = strdup_fixed(str, MAX_LINE);
}

static bool yesno(const char * s, ...) {
    int c = '\n';
    va_list ap;
    va_start(ap, s);

    vprintf(s, ap);
    char s1[MAX_LINE];


    while (getline(s1, MAX_LINE) > 0) {
        c = tolower(s1[0]);

        if (c == 'n' || c == 'y'||c == '\n'||c == '0')
            break;
        else
            vprintf(s, ap);
    }

    if (c == 'y' || c == '\n') {
        va_end(ap);
        return true;
    } else if ( c == 'n' || c == '0' ) {
        va_end(ap);
        return false;
    } else {
        printf("\n");
        va_end(ap);
        return false;
    }
}

//
//static bool yesno(const string s, ...)
//{
//    int c = '\n';
//    va_list ap;
//    va_start(ap, s);
//
//    vprintf(s, ap);
//    char s1[MAX_LINE];
//
//
//    while (getline(s1, MAX_LINE) > 0) {
//        c = tolower(s1[0]);
//
//        if (c == 'n' || c == 'y')
//            break;
//        else if (c == '\n') {
//            va_end(ap);
//            return true;
//            // printf(s,ap);
//        } else if (c == '0') {
//            va_end(ap);
//            return false;
//        }
//        else break;
//    }
//
//    if (c == 'y') {
//        va_end(ap);
//        return true;
//    } else if ( c == 'n' || c == '0' ) {
//        va_end(ap);
//        return false;
//    } else {
//        printf("\n");
//        va_end(ap);
//        return false;
//    }
//}

static bool readconf(void) {
    FILE *fp = fopen(CONF_FILE, "r");
    char *res = NULL;
    bool group_started = false;
    char group_name[CONF_MAX_LINE ] = {' '};
    char line[CONF_MAX_LINE ] = {' '}; //CONF_MAX_LINE  = 1000
    char tmp_line[CONF_MAX_LINE ] = {' '}; //CONF_MAX_LINE  = 1000

    if (fp == NULL) {
        return false;
    }
//    printf("\n Parsing configuration ... \n");
//    fflush(stdout);

    while (fp != NULL) {
        clearstr(line);// Added by dilomo to resolve a startup bug. 100807
        clearstr(tmp_line);


        if (fgets(tmp_line, CONF_MAX_LINE , fp) != NULL) {
            //skip comments
            if (strstr(tmp_line, "##")) {
                continue;
            }
            if (group_started) {
                /* copy the name of the group to the current line
                   and then use the line read*/
                strncat(line, group_name, CONF_MAX_LINE);
                /*skip whitespaces*/
                trim_start(' ', tmp_line);
                trim_start('\t', tmp_line);
                strncat(line, tmp_line, CONF_MAX_LINE);
            } else
                strncpy(line, tmp_line, CONF_MAX_LINE); /*just copy the line read */

            switch (findconf(line)) {
            case lines_per_page:
                if ((res = rval(line)) == NULL)
                    break;

                rowsperscroll = atoi(res);

                if (rowsperscroll < 0 || rowsperscroll > 200)
                    rowsperscroll = 23;

                break;
            case date_separator_char:
                if ((res = rval(line)) == NULL)
                    break;

                trim(' ', res);
                trim('"', res);
                *res = res[0];

                strncpy(&date_separator, res, 1);

                break;
            case value_precision:
                if ((res = rval(line)) == NULL)
                    break;

                precision = atoi(res);

                if (precision < 0 || precision > 6)
                    precision = 2; //default value
                break;
            case ex_all: //export all filename read
                if ((res = rval(line)) == NULL)
                    break;
                trim(' ', res);
                trim('"', res);
                if (strlen(res) > 0)// Added by dilomo to resolve a startup bug. 100807
                    strncpy(exall_filename, res, CONF_MAX_LINE - CONF_LVAL_MAX);
                break;
            case ex_in:
                if ((res = rval(line)) == NULL)
                    break;
                trim(' ', res);
                trim('"', res);
                if (strlen(res) > 0)// Added by dilomo to resolve a startup bug. 100807
                    strncpy(exi_filename, res, CONF_MAX_LINE - CONF_LVAL_MAX);
                break;
            case ex_out:
                if ((res = rval(line)) == NULL)
                    break;
                trim(' ', res);
                trim('"', res);
                if (strlen(res) > 0)// Added by dilomo to resolve a startup bug. 100807
                    strncpy(exo_filename, res, CONF_MAX_LINE - CONF_LVAL_MAX);
                break;
            case date_format:
                if ((res = rval(line)) == NULL)
                    break;
                int tmp = atoi(res);

                if (tmp < 0 || tmp > 1)
                    tmp = 0;
                date_iso_format = tmp;

                break;
            case current_db:
                if ((res = rval(line)) == NULL)
                    break;

                trim(' ', res);
                trim('"', res);
                if (strlen(res) > 0)// Added by dilomo to resolve a startup bug. 100807
                    strncpy(current_filename, res, CONF_MAX_LINE - CONF_LVAL_MAX);

                break;
            case filter_ex:
                if ((res = rval(line)) == NULL)
                    break;

                if (strlen(res) > 0) {// Added by dilomo to resolve a startup bug. 100807
                    strncpy(filter_expression, res, CONF_MAX_LINE - CONF_LVAL_MAX);
                    validfilter = parsefilter(filter_expression);
                }
                break;
            case set_ID_onstartup:
                if ((res = rval(line)) == NULL)
                    break;
                int id = atoi(res);

                if ( id < 0 || id > 1)
                    set_new_ID_onstartup = false;
                else
                    set_new_ID_onstartup = id;

                break;
            case sl_remember_last_shown:
                if ((res = rval(line)) == NULL)
                    break;
                int bol = atoi(res);

                if ( bol < 0 || bol > 1)
                    rem_listpos = false;
                else
                    rem_listpos = bol;

                break;
                /* === Issue 2008-02:1 ===
                   must be removed because on app load there may be change to the filter
                   and sort order (from v. 3.0) and incorrect info will be shown


                case sfl_remember_last_shown:
                if ((res = rval(line)) == NULL)
                    break;
                int bol1 = atoi(res);

                if ( bol1 < 0 || bol1 > 1)
                    rem_filt_listpos = false;
                else
                    rem_filt_listpos = bol1;

                break;
                */
            case export_max_width:
                if ((res = rval(line)) == NULL)
                    break;

                max_exportwidth = atoi(res);

                if (max_exportwidth < 45 || max_exportwidth > 300)
                    max_exportwidth = 66;

                break;
            case add_export_typestats:
                if ((res = rval(line)) == NULL)
                    break;

                int bol2 = 0;
                bol2 = atoi(res);

                if ( bol2 < 0 || bol2 > 1)
                    add_typestats_exp = false;
                else
                    add_typestats_exp = bol2;

                break;
            case import_withID:
                if ((res = rval(line)) == NULL)
                    break;
                int import_id = atoi(res);

                if ( import_id < 0 || import_id > 1)
                    import_ID = false;
                else
                    import_ID = import_id;

                break;
            case start_group:
                if (group_started)
                    break;

                group_started = true;
                clearstr(group_name);
                trim('\n',line);
                trim(' ',line);
                trim('<',line);
                trim('>',line);
                strcpy(group_name, line);
                strcat(group_name,":");
                break;
            case end_group:
                group_started = false;
                clearstr(group_name);
                break;
            default:
                break;

            }
        } else
            break;
    }

//    printf("Done\n");
    fclose(fp);
    return true;
}

char *
rval(char *s) {
    char *res = NULL;
    res = strstr(s, "=");
    res++;
    /*trim whitespaces*/
    while (*res == ' ')
        res++;
    /*remove \n at the end of the string*/
    res[strlen(res)-1] = '\0';
    return res;
}

static enum conftype findconf(char *s) {
    if (strstr(s, "app:use_iso_date_format"))
        return date_format;
    if (strstr(s, "app:last_database_used"))
        return current_db;
    if (strstr(s, "app:last_filter_expression"))
        return filter_ex;
    if (strstr(s, "app:set_new_IDs_on_appload"))
        return set_ID_onstartup;
    if (strstr(s, "app:date_separator_char"))
        return date_separator_char;
    if (strstr(s, "app:value_precision"))
        return value_precision;

    if (strstr(s, "showlist:lines_per_page"))
        return lines_per_page;
    if (strstr(s, "showlist:remember_last_shown"))
        return sl_remember_last_shown;
    /*  === Issue 2008-02:1 === */
//    if (strstr(s, "showfilteredlist:remember_last_shown"))
//        return sfl_remember_last_shown;

    if (strstr(s, "export:max_width"))
        return export_max_width;
    if (strstr(s, "export:add_typestats"))
        return add_export_typestats;
    if (strstr(s, "export:both_path"))
        return ex_all;
    if (strstr(s, "export:income_path"))
        return ex_in;
    if (strstr(s, "export:outcome_path"))
        return ex_out;


    if (strstr(s, "import:preserve_ID"))
        return import_withID;

    /*add settings in groups*/
    trim(' ',s);
    if (strstr(s, "<app>") )
        return start_group;
    if (strstr(s, "<showlist>") )
        return start_group;
    if (strstr(s, "<export>") )
        return start_group;
    if (strstr(s, "<import>") )
        return start_group;

    /*end gropu addition*/
    if (strstr(s, "</app>") )
        return end_group;
    if (strstr(s, "</showlist>") )
        return end_group;
    if (strstr(s, "</export>") )
        return end_group;
    if (strstr(s, "</import>") )
        return end_group;


    return none;
}

static char *getconfstr(enum conftype t) {
    char str[CONF_LVAL_MAX] = {' '};
    clearstr(str);

    switch (t) {

        /*export */
    case ex_all:
        strcpy(str, "export:both_path");
        break;
    case ex_in:
        strcpy(str, "export:income_path");
        break;
    case ex_out:
        strcpy(str, "export:outcome_path");
        break;
    case export_max_width:
        strcpy(str, "export:max_width");
        break;
    case add_export_typestats:
        strcpy(str, "export:add_typestats");
        break;

        /*import*/
    case import_withID:
        strcpy(str, "import:preserve_ID");
        break;

        /* app*/
    case date_format:
        strcpy(str, "app:use_iso_date_format");
        break;
    case date_separator_char:
        strcpy(str, "app:date_separator_char");
        break;
    case value_precision:
        strcpy(str, "app:value_precision");
        break;
    case current_db:
        strcpy(str, "app:last_database_used");
        break;
    case filter_ex:
        strcpy(str, "app:last_filter_expression");
        break;
    case set_ID_onstartup:
        strcpy(str, "app:set_new_IDs_on_appload");
        break;

        /* show list*/
    case lines_per_page:
        strcpy(str, "showlist:lines_per_page");
        break;
    case sl_remember_last_shown:
        strcpy(str, "showlist:remember_last_shown");
        break;
        /*  === Issue 2008-02:1 ===
        case sfl_remember_last_shown:
           strcpy(str, "showfilteredlist:remember_last_shown");
           break;
           */

    default:
        strcpy(str, "NONE");
        break;
    }
    return  strdupl(str);
}

static bool updateconf(char *lval, char *rval) {

    /*backup file to recover if errors popup*/
    char bname[strlen(CONF_FILE)+4];
    strcpy(bname, CONF_FILE);
    strcat(bname, ".b");
    copyfile(CONF_FILE, bname);
    char* lval_ingrp = NULL; //in group
    lval_ingrp = strchr(lval, ':');
    if (lval_ingrp != NULL)
        lval_ingrp++;

    bool updated = false;

    FILE *in =  fopen(CONF_FILE, "r");
    FILE *out = fopen("updcfg.tmp", "w");

    char line[CONF_MAX_LINE ];

    if ((in == NULL) || (out == NULL))
        return false;

    while (in != NULL) {
        if (fgets(line, CONF_MAX_LINE , in) != NULL) {
            if (strstr(line, lval)) {
                fprintf(out, "%s = %s\n", lval, rval);
                updated = true;
            } else if (lval_ingrp != NULL && strstr(line, lval_ingrp)) {
                fprintf(out, "%s = %s\n", lval_ingrp, rval);
                updated = true;
            } else
                fprintf(out, "%s", line);

        } else
            break;
    }

    if (updated == false) {
        //fprintf(out, "\n## Property used to store the current DB path\n");
        fprintf(out, "\n%s = %s\n", lval, rval);
    }


    if (remove("acc.conf") != 0) {
        copyfile(bname, CONF_FILE);
        remove("updcfg.tmp");
        return false;
    }
    if (rename("updcfg.tmp", CONF_FILE) != 0)
        return false;

    // clean up
    if (fclose(in) != 0 || fclose(out) != 0) {
        fprintf(stderr, "Error in closing files\n");
        return false;
    }

    free(lval);//because strdupl was used
    return true; //readconf();
}

static bool save_to_bin(char *filepath, char *mode) {
    /*save the contents of the db to binary file*/
    dbout = fopen(filepath, mode);
    if (dbout == NULL) {
        printe("Could not open file(mode:%s): %s)", mode, filepath);
        return false;
    }

    /*write income and outcome count in the begining
     *to enable read function to load the data properly*/
    Index tmp = count(income);
    fwrite(&tmp, sizeof(Index), 1, dbout);
    tmp = count(outcome);
    fwrite(&tmp, sizeof(Index), 1, dbout);
    tmp = 0;

    /*write number of last page shown */
    fwrite(&pageI, sizeof(Index), 1, dbout);
    fwrite(&pageO, sizeof(Index), 1, dbout);
    fwrite(&pageIf, sizeof(Index), 1, dbout);
    fwrite(&pageOf, sizeof(Index), 1, dbout);


    /*save last page ID scrolled in show list*/
    if (lastTraversenIb != NULL) {
        Index ltib = lastTraversenIb->data.ID;
        fwrite(&ltib, sizeof(Index), 1, dbout);
    } else {
        fwrite(&tmp, sizeof(Index), 1, dbout);
    }
    if (lastTraversenOb != NULL) {
        Index ltob = lastTraversenOb->data.ID;
        fwrite(&ltob, sizeof(Index), 1, dbout);
    } else {
        fwrite(&tmp, sizeof(Index), 1, dbout);
    }

    /*  === Issue 2008-02:1 ===
       write IDs to the last shown pages of show filtered list
       These should be retained in order to read existing db
    */
    if (lastTraversenIb_filtered != NULL) {
        Index ltibf = lastTraversenIb_filtered->data.ID;
        fwrite(&ltibf, sizeof(Index), 1, dbout);
    } else {
        fwrite(&tmp, sizeof(Index), 1, dbout);
    }
    if (lastTraversenOb_filtered != NULL) {
        Index ltobf = lastTraversenOb_filtered->data.ID;
        fwrite(&ltobf, sizeof(Index), 1, dbout);
    } else {
        fwrite(&tmp, sizeof(Index), 1, dbout);
    }

    fflush(dbout);

    /* save all records consequently
       == WARNING ==
       traverse should not be called on
       SaveItem if dbout is null
    */
    traverse(SaveItem, income);
    fflush(dbout);
    traverse(SaveItem, outcome);
    fflush(dbout);

    if (fclose(dbout))
        return false;

    haschanges = false;

    return true;
}

static void SaveItem(recdata rp) {
    //remove to optimise save speed
//    if (dbout == NULL)
//        return;

    fwrite(&rp, sizeof(rp), 1, dbout);

    /*save type as num preceded char seq*/
    size_t len = strlen(rp.type);

    /*first write the length of the string and then all the chars*/
    fwrite(&len, sizeof(size_t), 1, dbout);
    fwrite(rp.type, sizeof(char), len, dbout);

    len=0;
    if (rp.descr != NULL) {
        /*write description as num preceded seq*/
        len = strlen(rp.descr);
        fwrite(&len, sizeof(size_t), 1, dbout);
        fwrite(rp.descr, sizeof(char), len, dbout);
    } else
        fwrite(&len, sizeof(size_t), 1, dbout);
}

static bool save_to_txt(char *filepath, char *mode, rectype t, void (*func)(recdata r)) {
    dbout = fopen(filepath, mode);
    /*sort awarenes*/
    bool sort_on = (mSortType != SORT_BY_NONE && mSortOrder != NODIR) ? true : false;

    if (sort_on == true) {
        fill_sortarray(t);
        set_printFunc(SaveTxtItem);
    }

    /*====================*/

    if (dbout == NULL) {
        printe("Could not open file (mode:%s): %s)", mode, filepath);
        return false;
    }

    fprintheader(dbout, "ACCOUNTANT DATABASE REPORT" , true);

    /*save according to user choise*/
    switch (t) {
    case income:
        if (IncomesCount > 0) {
            calcidwidth(income);

            fprintf(dbout, "\nIncome list (%lu records):\n\n", count(income));

            if (add_typestats_exp == true) {
                fprintln(dbout, '-', max_exportwidth);
                /* TODO (toni#1#): If saves through filters only the
                  typesavailable in the filter list should be added to the statistic
                  if someday I change the funct there will occure a bug */

                if ((*func) == SaveFilteredTxtItem)
                    Traverse(&mTypesIncomesFiltered, fprintStat);
                else
                    Traverse(&mTypesIncomes, fprintStat);

                fprintln(dbout, '-', max_exportwidth);
                fprintf(dbout, "\n");
            }

            fprintf(dbout, "%*s %-10s\t%*s Type/Description\n", idwidth, "ID", "Date", 10, "Value");
            fprintln(dbout, '=', max_exportwidth);
            fprintf(dbout, "\n");

            if (sort_on == false )
                traverse((*func), income);
            else
                Traverse_SortArray_Default(sort_arrI, sort_arrI_num);
        } else
            fprintf(dbout, "No income records to export.\n");
        break;

    case outcome:
        if (OutcomesCount > 0) {
            calcidwidth(outcome);

            fprintf(dbout, "\n\nOutcome list (%lu records):\n\n", count(outcome));
            if (add_typestats_exp == true) {
                fprintln(dbout, '-', max_exportwidth);
                /* TODO (toni#1#): If saves through filters only the
                  typesavailable in the filter list should be added to the statistic
                  if someday I change the funct there will occure a bug */
                if ((*func) == SaveFilteredTxtItem)
                    Traverse(&mTypesOutcomesFiltered, fprintStat);
                else
                    Traverse(&mTypesOutcomes, fprintStat);
                fprintln(dbout, '-', max_exportwidth);
                fprintf(dbout, "\n");
            }
            fprintf(dbout, "%*s %-10s\t%*s Type/Description\n", idwidth, "ID", "Date", 10, "Value");
            fprintln(dbout, '=', max_exportwidth);
            fprintf(dbout, "\n");

            if (sort_on == false )
                traverse((*func), outcome);
            else
                Traverse_SortArray_Default(sort_arrO, sort_arrO_num);
        } else
            fprintf(dbout, "\nNo outcome records to export.\n");
        break;

    case both:
        if (IncomesCount  > 0) {
            calcidwidth(income);

            fprintf(dbout, "\nIncome list (%lu records):\n\n", count(income));
            if (add_typestats_exp == true) {
                fprintln(dbout, '-', max_exportwidth);
                /* TODO (toni#1#): If saves through filters only the
                  typesavailable in the filter list should be added to the statistic
                  if someday I change the funct there will occure a bug */
                if ((*func) == SaveFilteredTxtItem)
                    Traverse(&mTypesIncomesFiltered, fprintStat);
                else
                    Traverse(&mTypesIncomes, fprintStat);

                fprintln(dbout, '-', max_exportwidth);
                fprintf(dbout, "\n");
            }
            fprintf(dbout, "%*s %-10s\t%*s Type/Description\n", idwidth, "ID", "Date", 10, "Value");
            fprintln(dbout, '=', max_exportwidth);
            fprintf(dbout, "\n");

            if (sort_on == false )
                traverse((*func), income);
            else
                Traverse_SortArray_Default(sort_arrI, sort_arrI_num);
        } else
            fprintf(dbout, "No income records to export.\n");

        if (OutcomesCount > 0) {
            calcidwidth(outcome);

            fprintf(dbout, "\n\nOutcome list (%lu records):\n\n", count(outcome));
            if (add_typestats_exp == true) {
                fprintln(dbout, '-', max_exportwidth);
                /* TODO (toni#1#): If saves through filters only the
                 typesavailable in the filter list should be added to the statistic
                 if someday I change the funct there will occure a bug */
                if ((*func) == SaveFilteredTxtItem)
                    Traverse(&mTypesOutcomesFiltered, fprintStat);
                else
                    Traverse(&mTypesOutcomes, fprintStat);

                fprintln(dbout, '-', max_exportwidth);
                fprintf(dbout, "\n");
            }
            fprintf(dbout, "%*s %-10s\t%*s Type/Description\n", idwidth, "ID", "Date", 10, "Value");
            fprintln(dbout, '=', max_exportwidth);
            fprintf(dbout, "\n");

            if (sort_on == false )
                traverse((*func), outcome);
            else
                Traverse_SortArray_Default(sort_arrO, sort_arrO_num);
        } else
            fprintf(dbout, "\nNo outcome records to export.\n");

        break;
    default:
        if (sort_on == true)
            set_printFunc(PrintItem);
        fclose(dbout);
        return false;
    }
    /*restore previous state*/
    if (sort_on == true)
        set_printFunc(PrintItem);


    fclose(dbout);
    return true;
}

static void SaveTxtItem(recdata rp) {
    if (dbout == NULL)
        return;

    size_t start = 0;

    start += fprintf(dbout, "%*lu ", idwidth , rp.ID);

    /*print customized date*/
    if (date_iso_format) {
        if (rp.date.day < 10)
            start += fprintf(dbout, "0%i%c", rp.date.day, date_separator);
        else
            start += fprintf(dbout, "%i%c", rp.date.day, date_separator);

        if (rp.date.month < 10)
            start += fprintf(dbout, "0%i%c", rp.date.month, date_separator);
        else
            start += fprintf(dbout, "%i%c", rp.date.month, date_separator);

        start += fprintf(dbout, "%i\t", rp.date.year);

    } else {
        start += fprintf(dbout, "%i%c", rp.date.year, date_separator);

        if (rp.date.month < 10)
            start += fprintf(dbout, "0%i%c", rp.date.month, date_separator);
        else
            start += fprintf(dbout, "%i%c", rp.date.month, date_separator);

        if (rp.date.day < 10)
            start += fprintf(dbout, "0%i\t", rp.date.day);
        else
            start += fprintf(dbout, "%i\t", rp.date.day);

    }

    // start += fprintf(dbout, "%i%c%i%c%i\t", rp.date.year, date_separator, rp.date.month, date_separator, rp.date.day);
    start += fprintf(dbout, "%10.*f ", precision, rp.value);

    /* add 2 beacause of thw whitespace and the point in the nums*/
    size_t chars = start /*+ precision*/ + 2;
    char  type_descr[MAX_LINE] = {'\0'} ;

    if (rp.descr != NULL) {
        sprintf(type_descr, "[%s] %s", rp.type, rp.descr);
    } else {
        sprintf(type_descr, "[%s]", rp.type);
    }

    if (max_exportwidth - chars <= strlen(type_descr))
        fprintcolumn(dbout, type_descr, chars, max_exportwidth);
    else
        fprintf(dbout, "%s|\n", type_descr);
}

static void SaveFilteredTxtItem(recdata rp) {
    if (passfilter(rp))
        SaveTxtItem(rp);
    else return;
}


/*XML Export*/

static bool save_to_XML(char *filepath, rectype t, void (*func)(recdata r)) {

    if (!InitXmlWriter(&xml_w, filepath))
        return false;

    char text[MAX_LINE];
    clearstrn(text, MAX_LINE);

    SetXmlWriterFormatting(&xml_w, ' ', 4);

    /*sort awarenes*/
    bool sort_on = (mSortType != SORT_BY_NONE && mSortOrder != NODIR) ? true : false;

    if (sort_on == true) {
        fill_sortarray(t);
        set_printFunc(SaveXMLItem);
    }

    /*====================*/
    /*start with the xml header*/
    WriteStartDoc(&xml_w);
    WriteStartElem(&xml_w, XML_ROOT_NAME);
    WriteAttributeString(&xml_w, XML_APP_VER, (char *) shortver);

    ultoa(IncomesCount, text);
    WriteAttributeString(&xml_w, "Incomes", text);
    clearstr(text);
    ultoa(OutcomesCount, text);
    WriteAttributeString(&xml_w, "Outcomes", text);

    /*save according to user choise*/
    switch (t) {
    case income:
        if (IncomesCount > 0) {
            /*TODO - add to xml
              if (add_typestats_exp == true) {
                  Traverse(&mTypesIncomes, fprintStat);
              }
              */

            if (sort_on == false )
                traverse((*func), income);
            else
                Traverse_SortArray_Default(sort_arrI, sort_arrI_num);
        }
        break;

    case outcome:
        if (OutcomesCount > 0) {
            /*TODO - add to xml
             if (add_typestats_exp == true) {
                 Traverse(&mTypesOutcomes, fprintStat);
             }
             */

            if (sort_on == false )
                traverse((*func), outcome);
            else
                Traverse_SortArray_Default(sort_arrO, sort_arrO_num);
        }
        break;

    case both:
        if (IncomesCount  > 0) {
            WriteStartElem(&xml_w, "Incomes");
            /*TODO - add to xml
            if (add_typestats_exp == true) {
                fprintln(dbout, '-', max_exportwidth);
                Traverse(&mTypesIncomes, fprintStat);
                fprintln(dbout, '-', max_exportwidth);
                fprintf(dbout, "\n");
            }
            */

            if (sort_on == false )
                traverse((*func), income);
            else
                Traverse_SortArray_Default(sort_arrI, sort_arrI_num);
            WriteEndElem(&xml_w);
        }

        if (OutcomesCount > 0) {
            WriteStartElem(&xml_w, "Outcomes");
            /*TODO - add to xml
            if (add_typestats_exp == true) {
                fprintln(dbout, '-', max_exportwidth);
                Traverse(&mTypesOutcomes, fprintStat);
                fprintln(dbout, '-', max_exportwidth);
                fprintf(dbout, "\n");
            }
            */

            if (sort_on == false )
                traverse((*func), outcome);
            else
                Traverse_SortArray_Default(sort_arrO, sort_arrO_num);

            WriteEndElem(&xml_w);
        }

        break;
    default:
        if (sort_on == true)
            set_printFunc(PrintItem);

        return false;
    }
    /*restore previous state*/
    if (sort_on == true)
        set_printFunc(PrintItem);


    WriteEndDoc(&xml_w);
    CloseXmlWriter(&xml_w);
    return true;
}

static void SaveXMLItem(recdata rp) {
    if (IsClosed(&xml_w))
        return;

    char text[MAX_LINE];
    clearstrn(text, MAX_LINE);

    /*write the start of the element and ID attr*/
    WriteStartElem(&xml_w, XML_REC_NAME);
    ultoa(rp.ID, text);
    WriteAttributeString(&xml_w, XML_ID_NAME, text);

    /*save date as subelement*/
    /*print customized date*/
    clearstr(text);
    if (date_iso_format) {
        char str1[50];
        clearstrn(str1, 50);
        char str2[50];
        clearstrn(str2, 50 );
        char str3[50];
        clearstrn(str3, 50);

        if (rp.date.day < 10)
            sprintf(str1, "0%i%c", rp.date.day, date_separator);
        else
            sprintf(str1, "%i%c", rp.date.day, date_separator);

        if (rp.date.month < 10)
            sprintf(str2, "0%i%c", rp.date.month, date_separator);
        else
            sprintf(str2, "%i%c", rp.date.month, date_separator);

        sprintf(str3, "%i", rp.date.year);

        strcat(text, str1);
        strcat(text, str2);
        strcat(text, str3);
    } else {
        char str1[50];
        clearstrn(str1, 50);
        char str2[50];
        clearstrn(str2, 50);
        char str3[50];
        clearstrn(str3, 50);

        sprintf(str1, "%i%c", rp.date.year, date_separator);

        if (rp.date.month < 10)
            sprintf(str2, "0%i%c", rp.date.month, date_separator);
        else
            sprintf(str2, "%i%c", rp.date.month, date_separator);

        if (rp.date.day < 10)
            sprintf(str3, "0%i", rp.date.day);
        else
            sprintf(str3, "%i", rp.date.day);

        strcat(text, str1);
        strcat(text, str2);
        strcat(text, str3);
    }

    WriteElementString(&xml_w, XML_DATE_NAME, text);

    /*print value*/
    clearstrn(text, MAX_LINE);
    sprintf(text, "%0.*f", precision, rp.value);
    WriteElementString(&xml_w, XML_VALUE_NAME, text);

    /*write type and description*/
    WriteElementString(&xml_w, XML_TYPE_NAME, rp.type);

    if (rp.descr != NULL)
        WriteElementString(&xml_w, XML_DESCR_NAME, rp.descr);
    else
        WriteElementString(&xml_w, XML_DESCR_NAME, "");

    WriteEndElem(&xml_w);
}

static void SaveFilteredXMLItem(recdata rp) {
    if (passfilter(rp))
        SaveXMLItem(rp);
    else return;
}


static bool load_to_app(char *filepath, char *mode) {
    dbin = fopen(filepath , mode);
    if (dbin == NULL) {
        printe("Failed load.\n Could not open the db file with mode (%s). At load_to_app(char *filepath, char *mode)\n", mode);
        return false;
    }

    size_t bytes;
    Index incount, outcount, lastTI, lastTO, lastTIf, lastTOf;
    recdata temp;
    Index i = 0;
    size_t tnum = 0;
    char tstr[MAX_LINE];
    clearstrn(tstr, MAX_LINE);
    int dummy;

//{ Other settings

    fread(&incount, sizeof(Index), 1, dbin);
    fread(&outcount, sizeof(Index), 1, dbin);

    if (rem_listpos) {
        fread(&pageI, sizeof(int), 1, dbin);
        fread(&pageO, sizeof(int), 1, dbin);
    } else {
        fread(&dummy, sizeof(int), 1, dbin);
        fread(&dummy, sizeof(int), 1, dbin);
    }
    if (rem_filt_listpos) {
        fread(&pageIf, sizeof(int), 1, dbin);
        fread(&pageOf, sizeof(int), 1, dbin);
    } else {
        fread(&dummy, sizeof(int), 1, dbin);
        fread(&dummy, sizeof(int), 1, dbin);
    }

    fread(&lastTI, sizeof(Index), 1, dbin);
    fread(&lastTO, sizeof(Index), 1, dbin);
    /*  === Issue 2008-02:1 === */
    fread(&lastTIf, sizeof(Index), 1, dbin);
    fread(&lastTOf, sizeof(Index), 1, dbin);


    //printf("%lu %lu\n",incount,outcount);

//}

    while ((bytes = fread(&temp, sizeof(recdata), 1 , dbin)) > 0) {
        if (i < incount) {
            clearstr(tstr);
            /*read type */
            fread(&tnum, sizeof(size_t), 1, dbin);
            fread(tstr, sizeof(char), tnum, dbin);
            temp.type = strdup_fixed(tstr, tnum);
            if (temp.type == NULL) {
                printi(" Database corrupted at income row No. %lu\n Load failed.\n", i);
                printe(" Database corrupted at income row No. %lu\n Load failed.\n", i);
                return false;
            }
            /*save type for autocompletion*/
            CItem * pitm;
            CItem titm;
            strcpy(titm.text, temp.type);
            titm.total = temp.value;
            titm.num = 1;
            if ((pitm = FindItem(titm, &mTypesIncomes, cmpCItems)) == NULL)
                AddItem(titm, &mTypesIncomes);
            else {
                pitm->total += temp.value;
                pitm->num += 1;
            }

            /*read description*/
            fread(&tnum, sizeof(size_t), 1, dbin);
            if (tnum > 0 ) {
                clearstr(tstr);
                fread(tstr, sizeof(char), tnum, dbin);

                temp.descr = strdup_fixed(tstr, tnum);
            } else
                temp.descr = NULL;

            if (set_new_ID_onstartup)
                addtolist(temp, income);
            else
                addtolist_withID(temp, income);
        } else {
            clearstr(tstr);
            /*read type */
            fread(&tnum, sizeof(size_t), 1, dbin);
            fread(tstr, sizeof(char), tnum, dbin);
            temp.type = strdup_fixed(tstr, tnum);
            if (temp.type == NULL) {
                printi(" Database corrupted at income row No. %lu\n Load failed.\n", i-incount);
                printe(" Database corrupted at income row No. %lu\n Load failed.\n", i-incount);
                return false;
            }
            /*save type for autocompletion*/
            CItem * pitm;
            CItem titm;
            strcpy(titm.text, temp.type);
            titm.total = temp.value;
            titm.num = 1;
            if ((pitm = FindItem(titm, &mTypesOutcomes, cmpCItems)) == NULL)
                AddItem(titm, &mTypesOutcomes);
            else {
                pitm->total += temp.value;
                pitm->num += 1;
            }

            /*read description*/
            fread(&tnum, sizeof(size_t), 1, dbin);

            if (tnum > 0 ) {
                clearstr(tstr);
                fread(tstr, sizeof(char), tnum, dbin);

                temp.descr = strdup_fixed(tstr, tnum);
            } else
                temp.descr = NULL;

            if (set_new_ID_onstartup)
                addtolist(temp, outcome);
            else
                addtolist_withID(temp, outcome);
        }
        i++;
    }
    /*test read*/
    assert(i == (incount + outcount));

    if (rem_listpos) {
        lastTraversenIb = findrec_ID(lastTI, income);
        if (lastTraversenIb == NULL)
            pageI = 1;
        lastTraversenOb = findrec_ID(lastTO, outcome);
        if (lastTraversenOb == NULL)
            pageO = 1;
    }

    /*  === Issue 2008-02:1 === */
    if (rem_filt_listpos) {
        lastTraversenIb_filtered = findrec_ID(lastTIf, income);
        if (lastTraversenIb_filtered == NULL)
            pageIf = 1;
        lastTraversenOb_filtered = findrec_ID(lastTOf, outcome);
        if (lastTraversenOb_filtered == NULL)
            pageOf = 1;
    }

    if (fclose(dbin))
        return false;

    return true;
}

static bool merge_db(FILE * mergefile) {
    if (mergefile == NULL) {
        printe(" Could not open the database file to merge. At merge_db(FILE * mergefile)\n");
        return false;
    }

    size_t bytes;
    Index incount, outcount, dummy;
    recdata temp;
    Index i = 0;
    size_t tnum = 0;
    char tstr[MAX_LINE];
    clearstrn(tstr, MAX_LINE);

    fread(&incount, sizeof(Index), 1, mergefile);
    fread(&outcount, sizeof(Index), 1, mergefile);

    /*skip the unnecessary fields*/
    fread(&dummy, sizeof(int), 1, mergefile);
    fread(&dummy, sizeof(int), 1, mergefile);
    fread(&dummy, sizeof(int), 1, mergefile);
    fread(&dummy, sizeof(int), 1, mergefile);

    fread(&dummy, sizeof(Index), 1, mergefile);
    fread(&dummy, sizeof(Index), 1, mergefile);
    fread(&dummy, sizeof(Index), 1, mergefile);
    fread(&dummy, sizeof(Index), 1, mergefile);

    //printf("%lu %lu\n",incount,outcount);
    while ((bytes = fread(&temp, sizeof(recdata), 1 , mergefile)) > 0) {
        //     clearstr(tstr);
        if (i < incount) {
            clearstr(tstr);
            /*read type */
            fread(&tnum, sizeof(size_t), 1, mergefile);
            fread(tstr, sizeof(char), tnum, mergefile);
            temp.type = strdup_fixed(tstr, tnum);
            if (temp.type == NULL) {
                printi(" Database corrupted at income row No. %lu\n Load failed.\n", i);
                printe(" Database corrupted at income row No. %lu\n Load failed.\n", i);
                return false;
            }

            /*save type for autocompletion*/
            CItem titm;
            strcpy(titm.text, temp.type);
            if (!ContainsItem(titm, &mTypesIncomes, cmpCItems))
                AddItem(titm, &mTypesIncomes);

            /*read description if any. use the same variables as in type*/
            fread(&tnum, sizeof(size_t), 1, mergefile);
            if (tnum > 0 ) {
                clearstr(tstr);
                fread(tstr, sizeof(char), tnum, mergefile);
                temp.descr = strdup_fixed(tstr, tnum);
            } else //no description
                temp.descr = NULL;

            addtolist(temp, income);
        } else {
            clearstr(tstr);
            /*read description or type */
            fread(&tnum, sizeof(size_t), 1, mergefile);
            fread(tstr, sizeof(char), tnum , mergefile);
            temp.type = strdup_fixed(tstr, tnum);
            if (temp.type == NULL) {
                printi(" Database corrupted at outcome row No. %lu\n Load failed.\n", i - incount);
                printe(" Database corrupted at outcome row No. %lu\n Load failed.\n", i - incount);
                return false;
            }

            /*save type for autocompletion*/
            CItem titm;
            strcpy(titm.text, temp.type);
            if (!ContainsItem(titm, &mTypesOutcomes, cmpCItems))
                AddItem(titm, &mTypesOutcomes);

            /*read description if any. use the same variables as in type*/
            fread(&tnum, sizeof(size_t), 1, mergefile);
            if (tnum > 0 ) {
                clearstr(tstr);
                fread(tstr, sizeof(char), tnum, mergefile);
                temp.descr = strdup_fixed(tstr, tnum);
            } else //no description
                temp.descr = NULL;

            addtolist(temp, outcome);
        }
        /*increment the counter for incomes and outcomes*/
        i++;
    }

    if (fclose(mergefile))
        return false;

    return true;
}

static void clrscr() {
    int i;
    for (i = 0; i <= SCREEN_HEIGHT+6; i++)
        printf("\n");
}

static void calcidwidth(rectype t) {
    /*calculationg the ID width according to tha largest value*/
    char *cp = (char*) calloc(11, sizeof(char));//11 - space for unsigned long
    ultoa(lastIndex(t), cp);
    idwidth = strlen(cp);
    free(cp);
}

static void calcidwidth_rec(REC *r) {
    assert(r != NULL);

    long long int current = r->data.ID;
    /*estimate the width by the powers of 10 the number has*/
    for (idwidth = 0; current > 0; ++idwidth) {
        current /= 10;
    }
}

void printi(const char * fmt, ...) {
    hasmsg = true;
    va_list ap;
    va_start(ap, fmt);
    //clearstr(infomsg);
    vsnprintf(infomsg, INFO_LEN, fmt, ap);
    va_end(ap);
}

static void showinfo() {
    if (hasmsg == true) {
        printf(infomsg);
        hasmsg = false;
    }
}

static void checksave(void) {
    if (haschanges) {
        clrscr();
        if (yesno(" Current database has changes that will be\n lost after the action. Save now?[Y/n]")) {
            printf("\n Saveing database ... ");
            (save_to_bin(current_filename, "wb") == true ? printf("OK\n") : printf("FAILED\n")) ;
            haschanges = false;
            list_page_changed = false;
        }
    } else if (haschanges == false) {
        if (rem_listpos || rem_filt_listpos) {
            if (list_page_changed) {
                printf(" Saveing last list pages in database ... ");
                (save_to_bin(current_filename, "wb") == true ? printf("OK\n") : printf("FAILED\n")) ;
                list_page_changed = false;
            }
        }
    }
}

static char * readvalue(int maxlen, char *prompt) {
    int c = 0;
    printf("\n");
    printf(prompt);
    char s[maxlen];

    while ((c = getline(s, maxlen)) < 2)
        printf(prompt);

    if (s[0] == EOF || s[0] == '\0' ||s[0] == '0')
        return NULL; //exit on EOF

    s[c-1] = '\0';
    /*Quotes must be used to include ws*/
    trim(' ', s);
    return strdup_fixed(s, maxlen);
}

/*Description is NOT supported filter type and won't be soon*/
bool parsefilter(char *s) {
    int len = strlen(s);

    if (len == 0)
        return parsefilter(filter_expression); //if no string passed remain the old filter as valid

    int i, c, j, k;
    bool brokentype = false;
    /*strings must be initialized because strange results occure*/
    char type   [FILTER_TYPE_LEN + 1]       = {' '};
    char eq     [FILTER_EQ_LEN + 1]         = {' '};
    char val    [FILTER_UNKNOW_DATA_LEN + 1] = {' '};


    /*clear array with current filtes*/
    for (i = 0; i < FILTER_COUNT; i++) {
        filters[i].type = none;
        filters[i].eq = none;
    }
    /*clear filter error log*/
    for (i = 0; i < FILTERLOG_ROWS; i++)
        clearstr(filterlog[i]);

    clearstr(type);
    clearstr(eq);
    clearstr(val);

    i = j = k = currentrow = 0;

    for (k = 0; k < FILTER_COUNT; k++) {


        /*== parse parameter type ==*/
        for (; isspace(s[i]); i++)//skips empty spaces
            ;

        j = 0;
        while ((c = s[i++]) != '\0' && (isalpha(c) || c == FILTER_TYPE_CHAR) && j < FILTER_TYPE_LEN) {
            if (j == 0) {
                if (c != FILTER_TYPE_CHAR)//not allowed a type to start with other char
                    brokentype = true;    //continue to read in order to return the whole wrong value
            }
            type[j++] = c;
        }
        if (brokentype) {
            /*mark filter as invalid*/
            filters[k].type = none;
            brokentype = false;
        } else
            filters[k].type = getftype(type);

        if (filters[k].type == none) { //if filter is not valid form a error message and quit the function
            strcpy(filterlog[currentrow], "Type '");
            strcat(filterlog[currentrow], type);
            strcat(filterlog[currentrow], "' does not exists!\n Allowed types are: #ID, #DATE, #VALUE, #TYPE and #SORT.\n Example: '#ID >= 125 #VALUE < 1850 #DATE <> 2.7.2007'\n");
            currentrow++;
            return false;
        }
        /* decreace i because while( ... ) reads one more char
         * than neccesary to determine end of type
         */
        i--;


        /*== parse filter equation: <, >, <= ... ==*/

        for (; isspace(s[i]); i++)//skips empty spaces
            ;

        j = 0;
        while ((c = s[i++]) != '\0' && (c == '<' || c == '>' || c == '=' || c == '~' || c == '!') && j < FILTER_EQ_LEN) {
            eq[j++] = c;
        }

        filters[k].eq = geteqtype(filters[k].type, eq);

        if (filters[k].eq == none) { //if filter is not valid form a error message and quit the function
            strcpy(filterlog[currentrow], "The sign(s) '");
            strcat(filterlog[currentrow], eq);
            strcat(filterlog[currentrow], "' before position '");
            char ind[1000];
            itoa2(i, ind);
            strcat(filterlog[currentrow], ind);
            strcat(filterlog[currentrow], "' of the filter expression is(are) not permited!\n Allowed signs are: <, >, <=, >=, <>(not equal) and =.\n Only for #TYPE there are allowed the signs ~ (contains) and !~ (not contains).\n Example: '#ID >= 125 #VALUE < 1850 #DATE <> 2.7.2007'\n");
            currentrow++;
            return false;
        }
        i--;
        if (c == '\0')
            break;


        /*== parse value part: IDs, Dates, Values or Types ==*/
        for (; isspace(s[i]); i++)//skips empty spaces
            ;

        j = 0;
        while (((c = s[i++]) != '\0' && c != FILTER_TYPE_CHAR) && j < FILTER_UNKNOW_DATA_LEN ) {
            val[j++] = c;
        }
        /*set tha value according to the filter type and index*/
        setfilterval(k, val);


        if (filters[k].type == none) { //if filter is not valid form a error message and quit the function
            strcpy(filterlog[currentrow], "The value '");
            strcat(filterlog[currentrow], val);
            strcat(filterlog[currentrow], "' before position '");
            char ind[1000];
            itoa2(i, ind);
            strcat(filterlog[currentrow], ind);
            strcat(filterlog[currentrow],
                   "' of the filter expression\n is not valid! Allowed formats:\n \tDate: day/month/year and year/month/day. Valid separators: . - / \\ :\n \tCurrency: only digits without signs -> 256448.32\n Example: '#ID >= 125 #VALUE < 1850 #DATE <> 2-7-2007 #SORT = TYPE:ASC'\n");
            currentrow++;
            return false;
        }
        if (c == FILTER_TYPE_CHAR)
            i--;

        if (filters[k].type == SORT) { //dismiss it
            filters[k].type = none;
            k--;
        }


        /*== check for cmpetitive Items ==*/

        for (j = 0; j < k; j++)
            /*
             * the filter should be of the same type for
             * the functions below to work properly
             */
            if (filters[j].type == filters[k].type) {
                /* if the sign is diffrent from == go on*/
                if ((filters[j].eq != equal) && (filters[k].eq != equal)) {

                    if ((filters[j].eq == filters[k].eq)) {
                        /*
                         * if the filter's signs are the same and
                         * the values are not the same replace the
                         * old filter with the new one only if the
                         * signs are different than <>. If they are
                         * equal remove the last filter added
                         */
                        if (filterdatacmp(filters[j], filters[k]) != 0) {
                            /* The following code
                             * may be usefull if OR relation is available.
                             */
//                            if (filters[j].eq != not_equal){
                            filters[j] = filters[k];
                            k--;
                            break;
//                            }
//                            else continue;
                        } else {
                            k--;
                            break;
                        }
                    } else {
                        /*
                         * Check if the values and signs are compatiable
                         * and if so leave the newly added filter. If the
                         * filters are conflicting replace the old with the
                         * new one and return k with 1 position back
                         */
                        if (compswap(filters[j], filters[k]))
                            continue; //elements compatiable
                        else {
                            filters[j] = filters[k];
                            k--;
                            break;
                        }
                    }

                } else {
                    /*
                     * just replace the old specification if
                     * the signs are differen, else do nothing
                     * if the values are not equal. The following code
                     * may be usefull if OR relation is available.
                     */
//                    if ((filters[j].eq == filters[k].eq)) {
//                        if (filterdatacmp(filters[j],filters[k]) == 0){
//                            k--;
//                            break;
//                        }else continue;
//                    }else {
                    filters[j] = filters[k];
                    k--;
                    break;
//                    }
                }
            }

        if (c == '\0')
            break;

        /*clear used resources for the next loop*/
        clearstr(type);
        clearstr(eq);
        clearstr(val);
    }
    return true;
}

static enum filtertype getftype(char *s) {
    trim(' ', s);

    if (strstr(s, ID_))
        if (strlen(s) == strlen(ID_))
            return ID;
    if (strstr(s, VALUE_))
        if (strlen(s) == strlen(VALUE_))
            return VALUE;
    if (strstr(s, DATE_))
        if (strlen(s) == strlen(DATE_))
            return DATE;
    if (strstr(s, TYPE_))
        if (strlen(s) == strlen(TYPE_))
            return TYPE;
    if (strstr(s, SORT_))
        if (strlen(s) == strlen(SORT_))
            return SORT;

    return none;
}

static enum equation geteqtype(enum filtertype ft, char *s) {

    if (strlen(s) == 1) {
        if (strstr(s, "<"))
            return less_than;
        if (strstr(s, ">"))
            return greater_than;
        if (strstr(s, "="))
            return equal;
        if (ft == TYPE) { // this equation is not implemented for other types
            if (strstr(s, "~"))
                return contains;
        }
    } else if (strlen(s) == 2) {
        if (strstr(s, "<="))
            return less_than_or_equal;
        if (strstr(s, ">="))
            return greater_than_or_equal;
        if (strstr(s, "<>"))
            return not_equal;
        if (ft == TYPE) {// this equation is not implemented for other types
            if (strstr(s, "!~"))
                return not_contains;
        }
    }
    return none;
}

static void setfilterval(int i, char *type) {

    if (type == NULL || strlen(type) == 0) {
        filters[i].type = none;
        return;
    }

//    char c;
//    int k = strlen(type)-1;
//    /*remove whitespaces after value*/
//    while ((c = type[k]) != '\0' && c == ' ') {
//        type[k] = '\0';
//        k-=1;
//    }

    /*remove ws and quotes in value: version 0.2 and later*/
    switch (filters[i].type) {
    case ID:
        trim(' ', type);
        trim('"', type);
        /*
         * Values can end with non digits and and to be parsed successfully by the
         * other functions so onlydigits() was replaced with start_with_digits()
         * because of incorrect results.
         */
        if (!start_with_digits(type))
            filters[i].type = none;
        else
            filters[i].data.ID = atoul(type);
        break;
    case DATE:
        trim(' ', type);
        trim('"', type);
        date dt = filters[i].data.date;
        if (parsedate(type, &dt))
            filters[i].data.date = dt;
        else {
            filters[i].type = none;
        }
        break;
    case VALUE:
        trim(' ', type);
        trim('"', type);
        if (!start_with_digits(type))
            filters[i].type = none;
        else
            filters[i].data.value = atof(type);
        break;
    case TYPE:
        trim(' ', type);
        trim('"', type);
        clearstr(filters[i].data.type);//very important because without this many filters cannot be parsed
        strncpy(filters[i].data.type, type, FILTER_UNKNOW_DATA_LEN);
        break;
    case SORT: /*set sort params*/
        trim(' ', type);
        trim('"', type);
        /*split string into type:value*/
        char c;
        int k = 0, j = 0;
        char val    [FILTER_UNKNOW_DATA_LEN + 1] = {' '};
        clearstr(val);

        /*parse sort type param.*/
        while (((c = type[k++]) != '\0' && c != ':') && j < FILTER_UNKNOW_DATA_LEN ) {
            val[j++] = c;
        }
        /*in case none of the below elements is found*/
        SortType st = mSortType;
        mSortType = SORT_BY_NONE;

        if (strstr(val, "ID"))
            if (strlen(val) == strlen("ID"))
                mSortType = SORT_BY_ID;
        if (strstr(val, "VALUE"))
            if (strlen(val) == strlen("VALUE"))
                mSortType = SORT_BY_VALUE;
        if (strstr(val, "DATE"))
            if (strlen(val) == strlen("DATE"))
                mSortType = SORT_BY_DATE;
        if (strstr(val, "TYPE"))
            if (strlen(val) == strlen("TYPE"))
                mSortType = SORT_BY_TYPE;

        /*if sorttype is invalid then exit parse*/
        if (mSortType == SORT_BY_NONE) {
            /*preserve old value if no new is entered*/
            mSortType = st;
            filters[i].type = none;
            return;
        }

        /*parse sort direction.*/
        clearstr(val);
        j = 0;
        while ((c = type[k++]) != '\0' && j < FILTER_UNKNOW_DATA_LEN ) {
            val[j++] = c;
        }


        SortDirection so = mSortOrder;
        mSortOrder = NODIR; //no direction

        if (strstr(val, "ASC"))
            if (strlen(val) == strlen("ASC"))
                mSortOrder = ASC;
        if (strstr(val, "DESC"))
            if (strlen(val) == strlen("DESC"))
                mSortOrder = DESC;

        /*if sort order is invalid exit parse*/
        if (mSortOrder == NODIR) {
            mSortOrder = so;
            filters[i].type = none;
        }

        break;
    }
}

bool passfilter(recdata rd) {
    int k;
    for (k = 0; k < FILTER_COUNT; k++) {
        if (filters[k].type != none || filters[k].eq != none) {
            if (!applyfilter(rd, filters[k]))
                return false;
        }
    }
    return true;
}

bool applyfilter(recdata a, struct filter_element b) {
    switch (b.type) {
    case ID:
        switch (b.eq) {
        case less_than:
            if (a.ID >= b.data.ID)
                return false;
            break;
        case greater_than:
            if (a.ID <= b.data.ID)
                return false;
            break;
        case equal:
            if (a.ID != b.data.ID)
                return false;
            break;
        case not_equal:
            if (a.ID == b.data.ID)
                return false;
            break;
        case greater_than_or_equal:
            if (a.ID < b.data.ID)
                return false;
            break;
        case less_than_or_equal:
            if (a.ID > b.data.ID)
                return false;
            break;
        case contains://only for strings
            return false;
            break;
        case not_contains://only for strings
            return false;
            break;
        }
        break;
    case DATE:
        switch (b.eq) {
        case less_than:
            if (datecmp(a.date, b.data.date) >= 0)
                return false;
            break;
        case greater_than:
            if (datecmp(a.date, b.data.date) <= 0)
                return false;
            break;
        case equal:
            if (datecmp(a.date, b.data.date) != 0)
                return false;
            break;
        case not_equal:
            if (datecmp(a.date, b.data.date) == 0)
                return false;
            break;
        case greater_than_or_equal:
            if (datecmp(a.date, b.data.date) < 0)
                return false;
            break;
        case less_than_or_equal:
            if (datecmp(a.date, b.data.date) > 0)
                return false;
            break;
        case contains://only for strings
            return false;
            break;
        case not_contains://only for strings
            return false;
            break;
        }
        break;
    case VALUE:
        switch (b.eq) {
        case less_than:
            if (a.value >= b.data.value)
                return false;
            break;
        case greater_than:
            if (a.value <= b.data.value)
                return false;
            break;
        case equal:
            if (a.value != b.data.value)
                return false;
            break;
        case not_equal:
            if (a.value == b.data.value)
                return false;
            break;
        case greater_than_or_equal:
            if (a.value < b.data.value)
                return false;
            break;
        case less_than_or_equal:
            if (a.value > b.data.value)
                return false;
            break;
        case contains://only for strings
            return false;
            break;
        case not_contains://only for strings
            return false;
            break;
        }
        break;
    case TYPE:
        switch (b.eq) {
        case less_than:
            if (strcmp(a.type, b.data.type) >= 0)
                return false;
            break;
        case greater_than:
            if (strcmp(a.type, b.data.type) <= 0)
                return false;
            break;
        case equal:
            if (strcmp(a.type, b.data.type) != 0)
                return false;
            break;
        case not_equal:
            if (strcmp(a.type, b.data.type) == 0)
                return false;
            break;
        case greater_than_or_equal:
            if (strcmp(a.type, b.data.type) < 0)
                return false;
            break;
        case less_than_or_equal:
            if (strcmp(a.type, b.data.type) > 0)
                return false;
            break;
        case contains://only for strings
            if (strstr(a.type, b.data.type) == NULL)
                return false;
            break;
        case not_contains://only for strings
            if (strstr(a.type, b.data.type) != NULL)
                return false;
            break;
        }
        break;
    case SORT: //should never go up here
        break;
    }

    return true;
}

static void printfilterlog(char *newexp) {
    assert(filter_expression != NULL);

    if (currentrow >= 0 && currentrow < FILTERLOG_ROWS) {
        int i;
        printf(" Current filter expression:\n ");
        printf(strlen(filter_expression) > 0 ? filter_expression : "<no filter set>");
        if (newexp != NULL) {
            printf("\n New filter expression:\n ");
            printf(strlen(newexp) > 0 ? newexp : "<not set>");
        } else  printf("\n");

        for (i = 0; i < currentrow; i++) {
            printf("\n\n %s\n", filterlog[i]);
        }
    }
}

/*the two filter structures MUST be of the same type e.g. DATE or VALUE*/
static int filterdatacmp(struct filter_element a, struct filter_element b) {
    switch (a.type) {
    case ID:
        if (a.data.ID < b.data.ID)
            return -1;
        else if (a.data.ID > b.data.ID)
            return 1;
        else return 0;
        break;
    case DATE:
        return datecmp(a.data.date, b.data.date);
        break;
    case VALUE:
        if (a.data.value < b.data.value)
            return -1;
        else if (a.data.value > b.data.value)
            return 1;
        else return 0;
        break;
    case TYPE:
        return strcmp(a.data.type, b.data.type);
        break;
    case SORT: //should never reach this
        break;
    }
    return -2;
}

static bool compswap(struct filter_element a, struct filter_element b) {
    /* This function is custom comparison of two filters:
     * (filters[j].val (filters[k].eq/filters[j].eq) filters[k].val)
     * first with the sign of the second and then the first filter
     */

    bool acomp = false, bcomp = false;


    switch (a.type) {
    case ID:
        /*compare with the sign of b*/
        switch (b.eq) {
        case less_than:
            if (a.data.ID < b.data.ID)
                acomp = true;
            else acomp = false;
            break;
        case greater_than:
            if (a.data.ID > b.data.ID)
                acomp = true;
            else acomp = false;
            break;
        case equal:
            break;
        case not_equal:
            if (a.data.ID != b.data.ID)
                acomp = true;
            else acomp = false;

            break;
        case greater_than_or_equal:
            if (a.data.ID >= b.data.ID)
                acomp = true;
            else acomp = false;
            break;
        case less_than_or_equal:
            if (a.data.ID <= b.data.ID)
                acomp = true;
            else acomp = false;
            break;
        case contains:
            acomp = false;
            break;
        case not_contains:
            acomp = false;
            break;
        }
        /*now compare with the sign of 'a' and reversed positions*/
        switch (a.eq) {
        case less_than:
            if (b.data.ID < a.data.ID)
                bcomp = true;
            else bcomp = false;
            break;
        case greater_than:
            if (b.data.ID > a.data.ID)
                bcomp = true;
            else bcomp = false;
            break;
        case equal:
            break;
        case not_equal:
            if (b.data.ID != a.data.ID)
                bcomp = true;
            else bcomp = false;
            break;
        case greater_than_or_equal:
            if (b.data.ID >= a.data.ID)
                bcomp = true;
            else bcomp = false;
            break;
        case less_than_or_equal:
            if (b.data.ID <= a.data.ID)
                bcomp = true;
            else bcomp = false;
            break;
        case contains:
            bcomp = false;
            break;
        case not_contains:
            bcomp = false;
            break;
        }
        break;
    case DATE:
        switch (b.eq) {
        case less_than:
            if (datecmp(a.data.date, b.data.date) == -1)
                bcomp = true;
            else bcomp =  false;
            break;
        case greater_than:
            if (datecmp(a.data.date, b.data.date) == 1)
                bcomp = true;
            else bcomp = false;
            break;
        case equal:
            break;
        case not_equal:
            if (datecmp(a.data.date, b.data.date) != 1)
                bcomp = true;
            else bcomp = false;
            break;
        case greater_than_or_equal:
            if (datecmp(a.data.date, b.data.date) >= 0)
                bcomp =  true;
            else bcomp = false;
            break;
        case less_than_or_equal:
            if (datecmp(a.data.date, b.data.date) <= 0)
                bcomp =  true;
            else bcomp =  false;
            break;
        case contains:
            bcomp = false;
            break;
        case not_contains:
            bcomp = false;
            break;
        }
        switch (a.eq) {
        case less_than:
            if (datecmp(b.data.date, a.data.date) > 0)
                acomp = true;
            else acomp = false;
            break;
        case greater_than:
            if (datecmp(b.data.date, a.data.date) > 0)
                acomp = true;
            else acomp = false;
            break;
        case equal:
            break;
        case not_equal:
            if (datecmp(b.data.date, a.data.date) != 0)
                acomp = true;
            else acomp = false;
            break;
        case greater_than_or_equal:
            if (datecmp(b.data.date, a.data.date) >= 0)
                acomp = true;
            else acomp = false;
            break;
        case less_than_or_equal:
            if (datecmp(b.data.date, a.data.date) <= 0)
                acomp = true;
            else acomp = false;
            break;
        case contains:
            acomp = false;
            break;
        case not_contains:
            acomp = false;
            break;
        }
        break;
    case VALUE:
        switch (b.eq) {
        case less_than:
            if (a.data.value < b.data.value)
                acomp = true;
            else acomp = false;
            break;
        case greater_than:
            if (a.data.value > b.data.value)
                acomp = true;
            else acomp = false;
            break;
        case equal:
            break;
        case not_equal:
            if (a.data.value != b.data.value)
                acomp = true;
            else acomp = false;
            break;
        case greater_than_or_equal:
            if (a.data.value >= b.data.value)
                acomp = true;
            else acomp = false;
            break;
        case less_than_or_equal:
            if (a.data.value <= b.data.value)
                acomp = true;
            else acomp = false;
            break;
        case contains:
            acomp = false;
            break;
        case not_contains:
            acomp = false;
            break;
        }
        switch (a.eq) {
        case less_than:
            if (b.data.value < a.data.value)
                bcomp = true;
            else bcomp = false;
            break;
        case greater_than:
            if (b.data.value > a.data.value)
                bcomp = true;
            else bcomp = false;
            break;
        case equal:
            break;
        case not_equal:
            if (b.data.value != a.data.value)
                bcomp = true;
            else bcomp = false;
            break;
        case greater_than_or_equal:
            if (b.data.value >= a.data.value)
                bcomp = true;
            else bcomp = false;
            break;
        case less_than_or_equal:
            if (b.data.value <= a.data.value)
                bcomp = true;
            else bcomp = false;
            break;
        case contains:
            bcomp = false;
            break;
        case not_contains:
            bcomp = false;
            break;
        }
        break;
    case TYPE:
        switch (b.eq) {
        case less_than:
            if (strcmp(a.data.type, b.data.type) < 0)
                acomp = true;
            else acomp = false;
            break;
        case greater_than:
            if (strcmp(a.data.type, b.data.type) > 0)
                acomp = true;
            else acomp = false;
            break;
        case equal:
            break;
        case not_equal:
            if (strcmp(a.data.type, b.data.type) != 0)
                acomp = true;
            else acomp = false;
            break;
        case greater_than_or_equal:
            if (strcmp(a.data.type, b.data.type) >= 0)
                acomp = true;
            else acomp = false;
            break;
        case less_than_or_equal:
            if (strcmp(a.data.type, b.data.type) <= 0)
                acomp = true;
            else acomp = false;
            break;
            /* these I mark true because it is unknown wheather
             * the string will or won't contain both of them - a or(and) b
             */
        case contains:
            acomp = true;
            break;
        case not_contains:
            acomp = true;
            break;
        }
        switch (a.eq) {
        case less_than:
            if (strcmp(b.data.type, a.data.type) < 0)
                bcomp = true;
            else bcomp = false;
            break;
        case greater_than:
            if (strcmp(b.data.type, a.data.type) > 0)
                bcomp = true;
            else bcomp = false;
            break;
        case equal:
            break;
        case not_equal:
            if (strcmp(b.data.type, a.data.type) != 0)
                bcomp = true;
            else bcomp = false;
            break;
        case greater_than_or_equal:
            if (strcmp(b.data.type, a.data.type) >= 0)
                bcomp = true;
            else bcomp = false;
            break;
        case less_than_or_equal:
            if (strcmp(b.data.type, a.data.type) <= 0)
                bcomp = true;
            else bcomp = false;
            break;
            /* these I mark true because it is unknown wheather
             * the string will or won't contain both of them - a or(and) b
             */
        case contains:
            acomp = true;
            break;
        case not_contains:
            acomp = true;
            break;
        }
        break;
    case SORT: //should never reach this
        break;
    }

    if (a.eq == not_equal || b.eq == not_equal)
        return (acomp && bcomp);
    else
        return acomp;
}

void ShowList(void(*func)(recdata rp), rectype t) {
    clrscr();
    int current = 0;

    /*if configuration read is ok then use the custom settiong for lines per page*/
    if (rowsperscroll > 0) {
        Index count = (t == income ? IncomesCount : OutcomesCount);
        REC * pageend = (t == income ? lastTraversenIb : lastTraversenOb);
        long pagecount = count / rowsperscroll;
        bool first = true;

        if (count % rowsperscroll > 0)
            pagecount++;
        do {
            /* Change page numbers according to records type and position*/
            switch (t) {
            case income:
                if (islastIncome) {
                    if (!first) {
                        pageI = 1;
                        islastIncome = false;
                    }
                } else {
                    if (!first) pageI++;
                }
                break;
            case outcome:
                if (islastOutcome) {
                    if (!first) {
                        pageO = 1;
                        islastOutcome = false;
                    }
                } else {
                    if (!first) pageO++;
                }
                break;
            case both: //not valid
                return;
                break;
            }

            /*mark changes only if rem_listpos is true*/
            if (!first && rem_listpos == true)
                list_page_changed = true;

            /* jump ahead and determine the width of the ID column*/
            if (jump(&pageend, rowsperscroll, t) != NULL) {
                calcidwidth_rec(pageend);
                printf("%lu", pageend->data.ID);
            } else {//end reached
                pageend = getlast(t);
                calcidwidth_rec(pageend);
                /*initialize again*/
                pageend = getfirst(t);
            }

            /*show the records*/
            clrscr();
            if ((current = traversen((*func), t, rowsperscroll, first)) == 0) {
                printi(" No records to list.\n");
                break;
            }
            first = false;

        } while (yesno("Page %d of %d (%d records). Show next page?[Y/n]:", t == income ? pageI : pageO, pagecount,
                       rowsperscroll < current ? rowsperscroll : current));
    } else { /* show all records at once*/
        do {
            clrscr();
            calcidwidth(t);
            if (traverse((*func), t) == 0) {
                printi(" No records to list.\n");
                break;
            }

        } while (yesno("Show list again [Y/n]:"));
    }

}

void ShowFilteredList(bool(*func)(recdata rp), rectype t) {
    clrscr();
    int current = 0;
    Index sort_count = 0;
    bool sort_on = (mSortType != SORT_BY_NONE && mSortOrder != NODIR) ? true : false;

    if (sort_on == true)
        fill_sortarray(t);

    /*if configuration read is ok then use the custom setting for lines per page*/
    if (rowsperscroll > 0) {
        /*WARNING  unsafe code if  count_filteredI  is wrong*/
//        count_filteredI = count_filtered(income, passfilter);
//        count_filteredO = count_filtered(outcome, passfilter);
        Index count = t == income? count_filteredI: count_filteredO;
        long pagecount = count / rowsperscroll;

        bool first = true;
        REC * pageend = (t == income ? lastTraversenIb_filtered : lastTraversenOb_filtered);

        if (count % rowsperscroll > 0)
            pagecount++;

        do {
            /* Change page numbers according to records type and position*/
            switch (t) {
            case income:
                if (islastIncome_filtered) {
                    if (!first) {
                        pageIf = 1;
                        islastIncome_filtered = false;
                    }
                } else {
                    if (!first) pageIf++;
                }
                break;
            case outcome:
                if (islastOutcome_filtered) {
                    if (!first) {
                        pageOf = 1;
                        islastOutcome_filtered = false;
                    }
                } else {
                    if (!first) pageOf++;
                }
                break;
            case both: //not valid
                return;
                break;
            }
            if (!first && rem_filt_listpos == true)
                list_page_changed = true;

            clrscr();


            if (sort_on == false) {
                /* jump ahead and determine the width of the ID column*/
                if (jump(&pageend, rowsperscroll , t) != NULL) {
                    calcidwidth_rec(pageend);
                } else {//end reached
                    pageend = getlast(t);
                    calcidwidth_rec(pageend);
                    /*initialize again*/
                    pageend = getfirst(t);
                }

                if ((current = traversen_filtered((*func), t, rowsperscroll, first)) == 0) {
                    printi(" No records to list.\n");
                    break;
                }
            } else {
                /*calculate idwidth */
                SortArray temp = (t == income ? sort_arrI: sort_arrO);
                Index tmp_num = (t == income ? sort_arrI_num: sort_arrO_num);
                /*reset width and find the bigges one for the page that will be printed*/
                idwidth = 0;
                Traversen_SortArray(temp, tmp_num, find_idwidth, sort_count, rowsperscroll);

                /*int Traversen_SortArray_Default(const SortArray arr, unsigned long num, unsigned long start,  unsigned long count);*/
                if ((current = Traversen_SortArray_Default(temp, tmp_num, sort_count, rowsperscroll)) == 0) {
                    printi(" No records to list.\n");
                    break;
                }

                sort_count += current;

                if (sort_count >= count) {
                    sort_count = 0;
                    switch (t) {
                    case income:
                        islastIncome_filtered = true;
                        break;
                    case outcome:
                        islastOutcome_filtered = true;
                        break;
                    case both:
                        islastIncome_filtered = true;
                        islastOutcome_filtered = true;
                        break;
                    }
                }

                //  printf("%d of %d (%d)\n" , sort_count, count, current);
            }
            first = false;

        } while (yesno("Page %d of %d (%d records). Show next page?[Y/n]:", t == income ? pageIf : pageOf, pagecount,
                       rowsperscroll < current ? rowsperscroll : current));
    } else { /* show all records at once*/
        calcidwidth(t);
        if (sort_on == false) {
            do {
                clrscr();
                if (traverse_filetered((*func), t) == 0) {
                    printi(" No records to list.\n");
                    break;
                }

            } while (yesno("Show list again [Y/n]:"));
        } else {
            do {
                clrscr();
                if (Traverse_SortArray_Default((t == income ? sort_arrI: sort_arrO),
                                               (t == income ? sort_arrI_num: sort_arrO_num)) == 0) {
                    printi(" No records to list.\n");
                    break;
                }

            } while (yesno("Show list again [Y/n]:"));
        }
    }
}

static void find_idwidth(SortElement sl) {
    /*copy width*/
    int width = idwidth;
    /*find next element's one and compare*/
    calcidwidth_rec(sl.rec_ptr);
    if (width > idwidth)
        idwidth = width;
}

void PrintItem(recdata rp) {
    size_t chars = 0;

    chars += printf("%*lu ", idwidth , rp.ID);

    /*print customized date*/
    if (date_iso_format) {
        if (rp.date.day < 10)
            chars += printf("0%i%c", rp.date.day, date_separator);
        else
            chars += printf("%i%c", rp.date.day, date_separator);

        if (rp.date.month < 10)
            chars += printf( "0%i%c", rp.date.month, date_separator);
        else
            chars += printf( "%i%c", rp.date.month, date_separator);

        chars += printf( "%i\t", rp.date.year);

    } else {
        chars += printf( "%i%c", rp.date.year, date_separator);

        if (rp.date.month < 10)
            chars += printf( "0%i%c", rp.date.month, date_separator);
        else
            chars += printf( "%i%c", rp.date.month, date_separator);

        if (rp.date.day < 10)
            chars += printf( "0%i\t", rp.date.day);
        else
            chars += printf( "%i\t", rp.date.day);

    }

    chars += printf("%10.*f ", precision, rp.value);
    //chars += precision + 2;//one for the dot and one for the ws
    chars += 2;//one for the dot and one for the ws

    char type_descr[MAX_LINE] = {'\0'};

    /*MAY NEED CHANGE*/
    if (rp.descr != NULL) {
        sprintf(type_descr,"[%s] %s", rp.type, rp.descr);
    } else {
        sprintf(type_descr,"[%s]", rp.type);
    }

    if (SCREEN_WIDTH - chars <= strlen(type_descr))
        printcolumn(type_descr, chars, SCREEN_WIDTH);
    else
        printf("%s|\n", type_descr);
}

bool PrintFilteredItem(recdata rp) {
    if (passfilter(rp)) {
        PrintItem(rp);
        return (true);
    }
    return (false);
}
static bool AddToTypeStatsI(recdata rp) {
    if (passfilter(rp)) {
        /*add to filtered typestats*/
        CItem * pitm;
        CItem titm;
        strcpy(titm.text, rp.type);
        titm.total = rp.value;
        titm.num = 1;
        if ((pitm = FindItem(titm, &mTypesIncomesFiltered, cmpCItems)) == NULL)
            AddItem(titm, &mTypesIncomesFiltered);
        else {
            pitm->total += rp.value;
            pitm->num += 1;
        }
        return (true);
    }
    return (false);
}

static bool AddToTypeStatsO(recdata rp) {
    if (passfilter(rp)) {
        /*add to filtered typestats*/
        CItem * pitm;
        CItem titm;
        strcpy(titm.text, rp.type);
        titm.total = rp.value;
        titm.num = 1;
        if ((pitm = FindItem(titm, &mTypesOutcomesFiltered, cmpCItems)) == NULL)
            AddItem(titm, &mTypesOutcomesFiltered);
        else {
            pitm->total += rp.value;
            pitm->num += 1;
        }
        return (true);
    }
    return (false);
}
void check_show_filter(rectype t) {
    Index count = (t == income ? IncomesCount : OutcomesCount);


    if ((validfilter || (mSortType != SORT_BY_NONE && mSortOrder != NODIR)) && strlen(filter_expression) > 0) {
        if (count == 0) {
            printi(" No records to list.\n");
            return;
        }
        ShowFilteredList(PrintFilteredItem, t);
    } else {
        clrscr();
        if (count == 0) {
            printi(" No records to list.\n");
            return;
        }
        printfilterlog(NULL);
        if (yesno(" Filter is not valid or there is no filter set. Show all records?[Y/n]: "))
            ShowList(PrintItem, t);
    }
}

bool editrecord_seq(rectype t) {
    bool notfound = false;
    bool editok = false;
    Index I = 0;
    REC *result = NULL;

    do {
        char * new;
        clrscr();

        if (notfound)
            new = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "ID could not be found!\nPlease type it again or enter new one: "),
                  notfound = false;
        else {
            if (editok) {
                printf("Changes saved to record with ID = %lu.\n", I);
                editok = false;
            }
            new = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Type the ID of the record to edit, 0 to go back: ");
        }

        if (new == NULL) {
            free(new);
            return false;
        }

        I = atoul(new);

        if ((result = findrec_ID(I, t)) != NULL) {
            clrscr();
            recdata newrec = result->data;
            printf("\n");
            PrintItem(result->data);
            println('-', SCREEN_WIDTH);
            if (!yesno("Edit the record?[Y/n]: ")) {
                free(new);
                continue;
            }

            getvalue(&newrec.value);
            readline_th(&newrec.type, t);
            readdescr(&newrec.descr);
            getdate(&newrec.date);

            /*first free tha last used type string*/
            free(result->data.type);
            /*save changes*/
            result->data = newrec;

            /* update type stats*/
            CItem * pitm;
            CItem titm;
            strcpy(titm.text, newrec.type);
            titm.total = newrec.value;
            titm.num = 1;
            if ((pitm = FindItem(titm, t== income? &mTypesIncomes : &mTypesOutcomes, cmpCItems)) == NULL) {
                AddItem(titm, t== income? &mTypesIncomes : &mTypesOutcomes);
                /* TODO (toni#1#): Here is noticed
                    suspicious behaviour
                    Perform tests */

                /*update stats for old type*/
                clearstr(titm.text);
                strcpy(titm.text, result->data.type);
                titm.total = result->data.value;
                titm.num = 1;
                if ((pitm = FindItem(titm, t== income? &mTypesIncomes : &mTypesOutcomes, cmpCItems)) == NULL) {
                    printe("Edited record have no type linked with it");
                } else {
                    pitm->total -= result->data.value;
                    pitm->num -= 1;
                }
            } else {
                pitm->total += newrec.value;
                pitm->num += 1;
            }

            editok = true;
            haschanges = true;
        } else
            notfound = true;

        free(new);
    } while (true);

    return true;
}

bool deleterecord_seq(rectype t) {
    Index I = 0;
    REC *result = NULL;
    bool notfound = false;
    bool delerr = false;
    bool delok = false;

    do {
        char *new1;
        clrscr();
        if (notfound)
            new1 = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "ID could not be found!\nPlease type another ID: "),
                   notfound = false;
        else if (delerr) {
            printf("Record with ID = %lu could not be deleted!\nThe record have no type associated or there are no items to delete.\n", I);
            new1 = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Type the ID of the record to delete, 0 to go back: "),
                   delerr = false;
        } else {
            if (delok) {
                printf("Record with ID = %lu deleted.\n", I);
                delok = false;
            }
            new1 = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Type the ID of the record to delete, 0 to go back: ");
        }

        if (new1 == NULL) {
            free(new1);
            return false;
        }
        I = atoul(new1);

        if ((result = findrec_ID(I, t)) != NULL) {
            CItem * pitm;
            CItem titm;
            strcpy(titm.text, result->data.type);
            titm.total = result->data.value;
            titm.num = 1;

            clrscr();
            PrintItem(result->data);
            printf("--------------------------------------------\n");
            if (yesno("Do you want to delete the record with ID = %lu?[Y/n]: ", I)) {
                if (!rmfromlist(result, t)) {
                    delerr = true;
                } else {
                    delok = true;
                    haschanges = true;

                    /*remove the type from typelist*/
                    if ((pitm = FindItem(titm, t== income? &mTypesIncomes : &mTypesOutcomes, cmpCItems)) == NULL) {
                        /*impossible to have record without type added*/
                        printe("It is impossible to have record without type added");
                        delerr = true;
                    } else {
                        pitm->total -= titm.total;
                        pitm->num -= 1;
                    }

                }
            }
        } else
            notfound = true;

        free(new1);
    } while (true);

    return true;
}

bool saveAsDatabase(void) {
    bool result = false;

    clrscr();
    char *name = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Type new name for the database: ");

    if (name == NULL) {
        free(name);
        return result;
    }
    /*check and add extention if necessary*/
    trim('"', name);
    if (!strstr(name, ".adb"))
        strcat(name, ".adb");

    clrscr();
    printf("Saveing data, please wait ...\n");

    result = (save_to_bin(name, "wb") == true && updateconf(getconfstr(current_db), name) == true);

    if (result == true) {
        //open successfull
        strncpy(current_filename, name, CONF_MAX_LINE - CONF_LVAL_MAX);
    }

    free(name);

    return result;
}

bool makeCopyDatabase(void) {
    bool result = false;
    clrscr();

    char *name = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Enter name for the copy of the current database: ");

    if (name == NULL) {
        free(name);
        return result;
    }

    /*check and add extention if necessary*/
    trim('"', name);
    if (!strstr(name, ".adb"))
        strcat(name, ".adb");

    clrscr();
    printf("Saveing data, please wait ...\n");

    result = save_to_bin(name, "wb");

    free(name);

    return result;
}

static void clear(void) {
    /*used to get any remaining chars after menu choise*/
    getline(empty, 2);
}

static void fprintheader(FILE *out, char *header, bool print_datetime) {
    assert(header != NULL);
    assert(out != NULL);

    fprintf(out, " ~ %s ~\n\n", header);

    if (print_datetime) {
        /*print date to the file*/
        time_t *ctm = (time_t *) malloc(sizeof(time_t));
        time(ctm);
        struct tm *nowb =  localtime(ctm);
        struct tm now = *nowb;
        free(ctm);

        if (date_iso_format) {
            fprintf(out, "Created on ");
            if (now.tm_mday < 10)
                fprintf(out, "0%d%c", now.tm_mday, date_separator);
            else
                fprintf(out, "%d%c", now.tm_mday, date_separator);
            if (now.tm_mon+1 < 10)
                fprintf(out, "0%d%c%d ", now.tm_mon+1, date_separator, now.tm_year + 1900);
            else
                fprintf(out, "%d%c%d ", now.tm_mon+1, date_separator, now.tm_year + 1900);

            if (now.tm_hour < 10)
                fprintf(out, "0%d:", now.tm_hour);
            else
                fprintf(out, "%d:", now.tm_hour);
            if (now.tm_min < 10)
                fprintf(out, "0%d:", now.tm_min);
            else
                fprintf(out, "%d:", now.tm_min);
            if (now.tm_sec < 10)
                fprintf(out, "0%d \n\n", now.tm_sec);
            else
                fprintf(out, "%d \n\n", now.tm_sec);

        } else {
            fprintf(out, "Created on ");
            if (now.tm_mday < 10)
                fprintf(out, "0%d%c", now.tm_year + 1900, date_separator);
            else
                fprintf(out, "%d%c", now.tm_year + 1900, date_separator);

            if (now.tm_mon+1 < 10)
                fprintf(out, "0%d%c%d ", now.tm_mon+1, date_separator, now.tm_mday);
            else
                fprintf(out, "%d%c%d ", now.tm_mon+1, date_separator, now.tm_mday);

            if (now.tm_hour < 10)
                fprintf(out, "0%d:", now.tm_hour);
            else
                fprintf(out, "%d:", now.tm_hour);
            if (now.tm_min < 10)
                fprintf(out, "0%d:", now.tm_min);
            else
                fprintf(out, "%d:", now.tm_min);
            if (now.tm_sec < 10)
                fprintf(out, "0%d \n\n", now.tm_sec);
            else
                fprintf(out, "%d \n\n", now.tm_sec);
        }
    }

}

static void editfilter(void) {
    clrscr();
    char copyf[CONF_MAX_LINE-CONF_LVAL_MAX] = {' '};
    char *val = NULL;
    bool parseok = true;

enter_again:

    clrscr();

//    printf("%d, %d", mSortType, mSortOrder);

    if (!validfilter || !parseok)
        printfilterlog(copyf);
    else
        printf("Current filter: %s\n", strlen(filter_expression) > 0 ? filter_expression : "<no filter set>");

    val = readvalue(CONF_MAX_LINE - CONF_LVAL_MAX, "Type new filter expression: ");
    if (val == NULL) {
        free(val);
        return;
    }


    if (!(parseok = parsefilter(val))) {
        strcpy(copyf, val);
        free(val);
        goto enter_again;
    } else {
        strcpy(filter_expression, val);
        validfilter = true;

        if (updateconf(getconfstr(filter_ex), val)) {
            count_filteredI = count_filtered(income, passfilter);
            count_filteredO = count_filtered(outcome, passfilter);
            CollectTypeStats_filtered(both);
            printi(" Filter expression set properly.\n");
        } else
            printi(" Filter expression could not be set properly.\n The file 'acc.conf' could be missing or could not be accessed.\n");
    }
    free(val);
}

static void clearfilter() {
    clrscr();
    if (yesno("Current Filter:\n%s\n\nDo you want to remove the filter?[Y/n]: ", strlen(filter_expression) > 0 ? filter_expression : "<no filter set>")) {
        clearstr(filter_expression);
        printi(" Filter removed.\n");
        count_filteredI =0;
        count_filteredO =0;
        EmptyTheList(&mTypesIncomesFiltered);
        EmptyTheList(&mTypesOutcomesFiltered);
    }
}

static void getmonthly_balance(void) {
    /*clear previous data*/
    int i;
    for (i = 1; i < 13; i++) {
        monthly_balance_outcome[i] = 0;
        monthly_balance_income[i] = 0;
    }

    traverse(AddtoMonthlyBalanceI, income);
    traverse(AddtoMonthlyBalanceO, outcome);
}

static void AddtoMonthlyBalanceI(recdata r) {
    if (r.date.year == monthly_balance_year)
        monthly_balance_income[r.date.month] += r.value;
}

static void AddtoMonthlyBalanceO(recdata r) {
    if (r.date.year == monthly_balance_year)
        monthly_balance_outcome[r.date.month] += r.value;
}

/*noy intended for regular use*/
static bool get_typestats(void) {
    sort_arrI_num = count_filtered(income, passfilter);
    sort_arrO_num = count_filtered(outcome, passfilter);

    CreateSortArray(&sort_arrI, sort_arrI_num);
    CreateSortArray(&sort_arrO, sort_arrO_num);

    set_sortDirect(DESC);

    sort_arrI_index = 0;
    sort_arrO_index  = 0;

    traverse_filetered_raw(AddtoSortArrayI, income);
    traverse_filetered_raw(AddtoSortArrayO, outcome);

    QuckSort_Default(sort_arrI, sort_arrI_num);
    QuckSort_Default(sort_arrO, sort_arrO_num);

    return true;
}

static bool fill_sortarray(rectype t) {
    /*set sort direction from the global var*/
    set_sortDirect(mSortOrder);
    bool result = false;

    /*init ot reinitialize sort arrays sort_arrI(O)*/
    if (t == income) {
        sort_arrI_num = count_filtered(income, passfilter);
        result = CreateSortArray(&sort_arrI, sort_arrI_num);
        sort_arrI_index = 0;
        traverse_filetered_raw(AddtoSortArrayI, income);
        QuckSort_Default(sort_arrI, sort_arrI_num);
    } else if (t == outcome) {
        sort_arrO_num = count_filtered(outcome, passfilter);
        result = CreateSortArray(&sort_arrO, sort_arrO_num);
        sort_arrO_index  = 0;
        traverse_filetered_raw(AddtoSortArrayO, outcome);
        QuckSort_Default(sort_arrO, sort_arrO_num);
    } else if (t == both) {
        sort_arrI_num = count_filtered(income, passfilter);
        result = CreateSortArray(&sort_arrI, sort_arrI_num);
        sort_arrI_index = 0;
        traverse_filetered_raw(AddtoSortArrayI, income);
        QuckSort_Default(sort_arrI, sort_arrI_num);

        sort_arrO_num = count_filtered(outcome, passfilter);
        result = (CreateSortArray(&sort_arrO, sort_arrO_num) && result);
        sort_arrO_index  = 0;
        traverse_filetered_raw(AddtoSortArrayO, outcome);
        QuckSort_Default(sort_arrO, sort_arrO_num);
    }

    return result;
}

static bool AddtoSortArrayI(REC* r) {
    if (passfilter(r->data) == true) {

        if (sort_arrI == NULL)
            return false;

        sort_arrI[sort_arrI_index].type = mSortType;
        switch (mSortType) {
        case SORT_BY_ID:
            sort_arrI[sort_arrI_index].data.ID = r->data.ID;
            break;
        case SORT_BY_DATE:
            sort_arrI[sort_arrI_index].data.date = r->data.date;
            break;
        case SORT_BY_VALUE:
            sort_arrI[sort_arrI_index].data.value = r->data.value;
            break;
        case SORT_BY_TYPE:
            sort_arrI[sort_arrI_index].data.type = r->data.type;
            break;
        case SORT_BY_NONE:
            break;
        }
        sort_arrI[sort_arrI_index].rec_ptr = r;

        sort_arrI_index++;
        return true;
    } else return false;
}

static bool AddtoSortArrayO(REC* r) {
    if (passfilter(r->data) == true) {

        if (sort_arrO == NULL)
            return false;

        sort_arrO[sort_arrO_index].type = mSortType;
        switch (mSortType) {
        case SORT_BY_ID:
            sort_arrO[sort_arrO_index].data.ID = r->data.ID;
            break;
        case SORT_BY_DATE:
            sort_arrO[sort_arrO_index].data.date = r->data.date;
            break;
        case SORT_BY_VALUE:
            sort_arrO[sort_arrO_index].data.value = r->data.value;
            break;
        case SORT_BY_TYPE:
            sort_arrO[sort_arrO_index].data.type = r->data.type;
            break;
        case SORT_BY_NONE:
            break;
        }
        sort_arrO[sort_arrO_index].rec_ptr = r;

        sort_arrO_index++;
        return true;
    } else return false;
}

static bool CollectTypeStats_filtered(rectype t) {
    EmptyTheList(&mTypesIncomesFiltered);
    EmptyTheList(&mTypesOutcomesFiltered);

    InitializeListPtr(&mTypesIncomesFiltered);
    InitializeListPtr(&mTypesOutcomesFiltered);

    switch (t) {
    case income:
        traverse_filetered(AddToTypeStatsI, income);
        break;
    case outcome:
        traverse_filetered(AddToTypeStatsO, outcome);
        break;
    case both:
        traverse_filetered(AddToTypeStatsI, income);
        traverse_filetered(AddToTypeStatsO, outcome);
        break;
    }

    return true;
}


static void fprintcolumn(FILE *out, const char *s, int start, int width) {

    /*the text printed is aligned in the bounds start and width and the
     *words are printed whole not separated on two rows*/
    int col_w = width - start;
    char word[100];
    int i, remaining, wordlen, starti;
    size_t size = strlen(s);

    remaining = col_w;
    starti = 0;
    do {
        /*print any char different from letter*/
        while (isalnum(s[starti]) == false && s[starti] != '\0') {
            fprintf(out, "%c", s[starti]);
            starti++;
            remaining--;
        }
        /*get whole words and print them correctly*/
        wordlen = getsword(word, s, starti, 100);
        if (remaining > wordlen + 1) {
            fprintf(out, word);
            remaining -= wordlen;
            starti += wordlen;
        } else {
            fprintf(out, "\n");
            for (i = 0; i <= start; i++)
                fprintf(out, " ");
            remaining = col_w;
            fprintf(out, word);
            remaining -= wordlen;
            starti += wordlen;
        }
    } while (starti < size - 1);

    fprintf(out, "|\n");
}

static void printcolumn(const char *s, int start, int width) {
    fprintcolumn(stdout, s, start, width);
}

int cmpCItems(const CItem itemA, const CItem itemB) {
    return (strcmp(itemA.text,itemB.text));
}

void printCItem(CItem item) {
    if (item.num > 0)
        printf(" '%s'", item.text);
}

void printStat(CItem item) {
    if (item.num > 0)
        printf("[%s] - %10.*f (%lu items)\n", item.text, precision, item.total, item.num);
}

void fprintStat(CItem item) {
    if (item.num > 0)
        fprintf(dbout, "[%s] - %10.*f (%lu items)\n", item.text, precision, item.total, item.num);
}

void DummyPrint(recdata rp) {
    /*do nothing here*/
    /*used for skipping*/
}


static void PrintHelp(void) {
    int menuresult = 0;
    while ((menuresult = drawhelp()) != 0) {
        switch (menuresult) {
        case 0:
            break;
        default:
            printi(" Invalid menu number entred - %d\n", menuresult);
            break;
        }
    }
}

static int drawhelp(void) {
    int res = 0;
    clrscr();
    printmid("== Accountant Breif Help ==", SCREEN_WIDTH);
    printf("\n  Use the numbers in front of the menu elements to navigate through the program.");
    printf("You can type 0(zero) to go back anywhere in the application.[Y/n] means that the");
    printf("default choise is Y and if you press Enter or type Y, y, yes you will continue  ");
    printf("else if you type 0, N, n, no and press Enter you will go back. \n");
    printf("  When asked for file name please type the full path to the file e.g. export.txt");
    printf("or /home/me/accountant/databases/joe.adb. The last used DB is automatically     ");
    printf("loaded and ready to use. After a session, the data you have added or changed is ");
    printf("not automatically saved to the currently opened DB. You have to go to Save DB > ");
    printf("Update current to save it. If you forgot to save the data you'll be asked to do ");
    printf("so at exit.\n");
    printf("  Status information, if any, is show above the menu titles. It can tell you    ");
    printf("about the result of the requested action. If you are not sure of the meaning of ");
    printf("an info message do not hesitate to bother me.\n");
    printf("  You can also configure the application with a special file - 'acc.conf' that  ");
    printf("must be placed in the same directory in which the application is. For more help ");
    printf("see the files at the docs subdirectory. The docs fully describe everything.\n\n");
    printmid("Development: Anton Kerezov, 2008, e-mail: ankere@gmail.com", SCREEN_WIDTH);
    printf("\n 0. Back\n\n");

    do {
        printf("Chose entry: ");
        scanf("%i", &res);
    } while (res < 0 || res > 0);

    clear();
    return res;
}




