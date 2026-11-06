#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#ifdef _MSC_VER
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif
#else
#include <strings.h>
#endif

#define MAX_FLIGHTS 100
#define MAX_PASSENGERS 500
#define CODE_LEN 20

// ===== MAU SAC (ANSI) & KICH THUOC GIAO DIEN =====
#define C_RESET  "\033[0m"
#define C_BOLD   "\033[1m"
#define C_RED    "\033[31m"
#define C_GREEN  "\033[32m"
#define C_YELLOW "\033[33m"
#define C_BLUE   "\033[34m"
#define C_CYAN   "\033[36m"
#define C_WHITE  "\033[97m"
#define W 84   // Chieu rong trang

typedef struct {
    char code[CODE_LEN];
    char from[50];
    char to[50];
    char date[15]; 
    char time[10]; 
    double price;
    int capacity;
    int booked;
    int active;    
} Flight;

typedef struct {
    char ticket[20];
    char flightCode[CODE_LEN];
    char fullName[50];
    char phone[15];
    int active;    
} Passenger;

Flight flights[MAX_FLIGHTS];
int flightCount = 0;

Passenger passengers[MAX_PASSENGERS];
int passengerCount = 0;

// Khai báo nguyên mẫu hàm
void adminMenu(void);
void customerMenu(void);
int adminLogin(void);
void mainMenu(void);

void menu(void);
int validPhone(const char *phone);
int phoneExists(const char *phone);
int validDate(const char *date);
int validTime(const char *time);
int findFlight(const char *code);
int findPassenger(const char *ticket);
void readLine(const char *prompt, char *str, int size);
double readDouble(const char *prompt, double min);
int readInt(const char *prompt, int min, int max);

void printFlight(const Flight *f);
void addFlight(void);
void listFlights(void);
void searchFlights(void);
void sortFlights(void);
void bookTicket(void);
void cancelTicket(void);
void listTickets(void);
void searchTicket(void);
void updateFlight(void);
void deleteFlight(void);
void revenueReport(void);
void saveData(void);
void loadData(void);

// ================= HAM GIAO DIEN =================
static void msgErr(const char *s);
int adminLogin()
{
    char pass[30];

    readLine("Nhap mat khau Admin: ", pass, sizeof(pass));

    if(strcmp(pass,"admin123")==0)
        return 1;

    msgErr("Sai mat khau!");
    return 0;
}

static void enableAnsi(void)
{
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if(h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode))
        SetConsoleMode(h, mode | 0x0004); // ENABLE_VIRTUAL_TERMINAL_PROCESSING
#endif
}

static void clearScreen(void)
{
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

static void pauseScreen(void)
{
    char b[16];
    printf("\n" C_CYAN "  Nhan Enter de tiep tuc..." C_RESET);
    fgets(b, sizeof(b), stdin);
}

static void hline(char c)
{
    putchar('+');
    for(int i = 0; i < W - 2; i++) putchar(c);
    puts("+");
}

static void centered(const char *text, const char *color)
{
    int len = (int)strlen(text);
    int l = (W - 2 - len) / 2;
    int r = W - 2 - len - l;
    printf("|%*s%s%s%s%*s|\n", l, "", color, text, C_RESET, r, "");
}

static void banner(const char *title)
{
    hline('=');
    centered("VIETFLY AIRLINES", C_BOLD C_CYAN);
    centered(title, C_BOLD C_WHITE);
    hline('=');
}

static void sectionTitle(const char *title)
{
    printf("\n" C_BOLD C_BLUE ">> %s" C_RESET "\n", title);
}

static void msgOk(const char *s)   { printf("\n" C_GREEN  C_BOLD "[ OK ]  " C_RESET C_GREEN  "%s" C_RESET "\n", s); }
static void msgErr(const char *s)  { printf("\n" C_RED    C_BOLD "[ LOI ] " C_RESET C_RED    "%s" C_RESET "\n", s); }
static void msgInfo(const char *s) { printf("\n" C_YELLOW C_BOLD "[ ! ]   " C_RESET C_YELLOW "%s" C_RESET "\n", s); }

// Dinh dang tien: 1500000 -> 1,500,000
static void fmtMoney(double v, char *out, size_t size)
{
    char tmp[32];
    long long n = (long long)(v + 0.5);
    snprintf(tmp, sizeof(tmp), "%lld", n);
    int len = (int)strlen(tmp), j = 0;
    for(int i = 0; i < len && (size_t)j < size - 2; i++)
    {
        if(i > 0 && (len - i) % 3 == 0) out[j++] = ',';
        out[j++] = tmp[i];
    }
    out[j] = 0;
}

// Bang chuyen bay
static void tableLine(void)
{
    int w[] = {10, 13, 13, 12, 7, 12, 9};
    for(int i = 0; i < 7; i++)
    {
        putchar('+');
        for(int k = 0; k < w[i]; k++) putchar('-');
    }
    puts("+");
}

static void flightHeader(void)
{
    tableLine();
    printf("|" C_BOLD C_CYAN " %-8s " C_RESET "|" C_BOLD C_CYAN " %-11s " C_RESET "|" C_BOLD C_CYAN " %-11s "
           C_RESET "|" C_BOLD C_CYAN " %-10s " C_RESET "|" C_BOLD C_CYAN " %-5s " C_RESET "|" C_BOLD C_CYAN
           " %10s " C_RESET "|" C_BOLD C_CYAN " %-7s " C_RESET "|\n",
           "MA CB", "NOI DI", "NOI DEN", "NGAY BAY", "GIO", "GIA VE", "GHE");
    tableLine();
}

static void printTicketCard(const Passenger *p, const Flight *f)
{
    char route[110] = "-";
    char money[32] = "-";
    char when[40] = "-";
    if(f)
    {
        snprintf(route, sizeof(route), "%s -> %s", f->from, f->to);
        snprintf(when, sizeof(when), "%s %s", f->date, f->time);
        fmtMoney(f->price, money, sizeof(money));
        strcat(money, " VND");
    }
    printf("\n" C_CYAN "+------------------------------------------------+" C_RESET "\n");
    printf(C_CYAN "|" C_RESET C_BOLD "              VE MAY BAY DIEN TU                " C_RESET C_CYAN "|" C_RESET "\n");
    printf(C_CYAN "+------------------------------------------------+" C_RESET "\n");
    printf("| %-14s: " C_BOLD C_YELLOW "%-30.30s" C_RESET " |\n", "Ma ve", p->ticket);
    printf("| %-14s: %-30.30s |\n", "Chuyen bay", p->flightCode);
    printf("| %-14s: %-30.30s |\n", "Hanh trinh", route);
    printf("| %-14s: %-30.30s |\n", "Khoi hanh", when);
    printf("| %-14s: %-30.30s |\n", "Hanh khach", p->fullName);
    printf("| %-14s: %-30.30s |\n", "Dien thoai", p->phone);
    printf("| %-14s: " C_BOLD C_GREEN "%-30.30s" C_RESET " |\n", "Gia ve", money);
    printf(C_CYAN "+------------------------------------------------+" C_RESET "\n");
}

// ================= IMPLEMENTATION =================
void mainMenu()
{
    banner("HE THONG DAT VE MAY BAY");

    printf("\n");
    printf("1. Dang nhap Admin\n");
    printf("2. Khach hang dat ve\n");
    printf("0. Thoat\n");
}
void adminMenu()
{
    int choice;

    do
    {
        clearScreen();

        banner("ADMIN");

        printf("1. Them chuyen bay\n");
        printf("2. Danh sach chuyen bay\n");
        printf("3. Tim chuyen bay\n");
        printf("4. Sap xep chuyen bay\n");
        printf("5. Cap nhat chuyen bay\n");
        printf("6. Xoa chuyen bay\n");
        printf("7. Danh sach ve\n");
        printf("8. Tra cuu ve\n");
        printf("9. Doanh thu\n");
        printf("0. Quay lai\n");

        choice = readInt("Lua chon: ",0,9);

        switch(choice)
        {
            case 1:addFlight();break;
            case 2:listFlights();break;
            case 3:searchFlights();break;
            case 4:sortFlights();break;
            case 5:updateFlight();break;
            case 6:deleteFlight();break;
            case 7:listTickets();break;
            case 8:searchTicket();break;
            case 9:revenueReport();break;
        }

        if(choice!=0)
            pauseScreen();

    }while(choice!=0);
}
void customerMenu()
{
    int choice;

    do
    {
        clearScreen();

        banner("KHACH HANG");

        printf("1. Xem chuyen bay\n");
        printf("2. Tim chuyen bay\n");
        printf("3. Dat ve\n");
        printf("4. Tra cuu ve\n");
        printf("5. Huy ve\n");
        printf("0. Quay lai\n");

        choice = readInt("Lua chon: ",0,5);

        switch(choice)
        {
            case 1:listFlights();break;
            case 2:searchFlights();break;
            case 3:bookTicket();break;
            case 4:searchTicket();break;
            case 5:cancelTicket();break;
        }

        if(choice!=0)
            pauseScreen();

    }while(choice!=0);
}

void menu(void)
{
    int fc = 0, tc = 0;
    for(int i = 0; i < flightCount; i++) if(flights[i].active) fc++;
    for(int i = 0; i < passengerCount; i++) if(passengers[i].active) tc++;

    banner("HE THONG QUAN LY & DAT VE MAY BAY");

    char info[100];
    snprintf(info, sizeof(info), "Chuyen bay dang mo: %d   |   Ve da dat: %d", fc, tc);
    centered(info, C_YELLOW);
    hline('-');

    printf("|" C_BOLD C_BLUE " %-38s " C_RESET "|" C_BOLD C_BLUE " %-39s " C_RESET "|\n",
           "[ QUAN LY CHUYEN BAY ]", "[ DAT VE & BAO CAO ]");
    hline('-');

    int   ln[] = {1, 2, 3, 4, 5, 6};
    const char *lt[] = {"Them chuyen bay", "Xem danh sach chuyen bay", "Tim/loc chuyen bay",
                        "Sap xep chuyen bay", "Cap nhat chuyen bay", "Xoa chuyen bay"};
    int   rn[] = {7, 8, 9, 10, 11, 0};
    const char *rt[] = {"Dat ve", "Huy ve", "Xem danh sach ve", "Tim ve theo ma",
                        "Thong ke doanh thu", ""};

    for(int i = 0; i < 6; i++)
    {
        printf("|  " C_BOLD C_CYAN "%2d" C_RESET ". %-32s  |", ln[i], lt[i]);
        if(i < 5)
            printf("  " C_BOLD C_CYAN "%2d" C_RESET ". %-33s  |\n", rn[i], rt[i]);
        else
            printf("%41s|\n", "");
    }
    hline('-');
    printf("|  " C_BOLD C_RED " 0" C_RESET ". %-*s|\n", W - 8, "Luu va thoat");
    hline('=');
}

int validPhone(const char *phone)
{
    int len = strlen(phone);
    if(len < 10 || len > 11)
        return 0;
    for(int i = 0; i < len; i++)
    {
        if(!isdigit((unsigned char)phone[i]))
            return 0;
    }
    return 1;
}

int phoneExists(const char *phone)
{
    for(int i = 0; i < passengerCount; i++)
    {
        if(passengers[i].active && strcmp(passengers[i].phone, phone) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int validDate(const char *date) { return strlen(date) == 10; }

int validTime(const char *time) { 
    int h, m;
    if (sscanf(time, "%d:%d", &h, &m) != 2) return 0;
    if (h < 0 || h > 23 || m < 0 || m > 59) return 0;
    return 1;
}

int findFlight(const char *code) {
    for(int i = 0; i < flightCount; i++) {
        if(flights[i].active && strcmp(flights[i].code, code) == 0) return i;
    }
    return -1;
}

int findPassenger(const char *ticket) {
    for(int i = 0; i < passengerCount; i++) {
        if(passengers[i].active && strcmp(passengers[i].ticket, ticket) == 0) return i;
    }
    return -1;
}

void readLine(const char *prompt, char *str, int size) {
    printf(C_CYAN "  > " C_RESET "%s", prompt);
    if(fgets(str, size, stdin)) {
        str[strcspn(str, "\n")] = 0;
    }
}

double readDouble(const char *prompt, double min) {
    double val;
    char buffer[100];
    while(1) {
        readLine(prompt, buffer, sizeof(buffer));
        val = atof(buffer);
        if(val >= min) return val;
        char msg[80];
        snprintf(msg, sizeof(msg), "Gia tri khong hop le. Phai >= %.0f", min);
        msgErr(msg);
    }
}

int readInt(const char *prompt, int min, int max) {
    int val;
    char buffer[100];
    while(1) {
        readLine(prompt, buffer, sizeof(buffer));
        val = atoi(buffer);
        if(val >= min && val <= max) return val;
        char msg[80];
        snprintf(msg, sizeof(msg), "Gia tri khong hop le. Phai tu %d den %d", min, max);
        msgErr(msg);
    }
}

void printFlight(const Flight *f) {
    char money[32], seats[16];
    fmtMoney(f->price, money, sizeof(money));
    snprintf(seats, sizeof(seats), "%d/%d", f->booked, f->capacity);

    const char *sc = C_GREEN;
    if(f->booked >= f->capacity) sc = C_RED;
    else if(f->booked * 10 >= f->capacity * 8) sc = C_YELLOW;

    printf("| " C_BOLD "%-8.8s" C_RESET " | %-11.11s | %-11.11s | %-10.10s | %-5.5s | %10s | %s%-7s" C_RESET " |\n",
           f->code, f->from, f->to, f->date, f->time, money, sc, seats);
}

void addFlight(void) { 
    banner("THEM CHUYEN BAY MOI");
    if(flightCount >= MAX_FLIGHTS) {
        msgErr("Danh sach chuyen bay da day.");
        return;
    }
    sectionTitle("Nhap thong tin chuyen bay");
    Flight f;
    readLine("Ma chuyen bay: ", f.code, sizeof(f.code));
    readLine("Noi di: ", f.from, sizeof(f.from));
    readLine("Noi den: ", f.to, sizeof(f.to));
    
    do {
        readLine("Ngay bay (YYYY-MM-DD): ", f.date, sizeof(f.date));
        if(!validDate(f.date)) msgErr("Ngay khong hop le.");
    } while(!validDate(f.date));

    do {
        readLine("Gio bay (HH:MM): ", f.time, sizeof(f.time));
        if(!validTime(f.time)) msgErr("Gio khong hop le.");
    } while(!validTime(f.time));

    f.price = readDouble("Gia ve (VND): ", 1000);
    f.capacity = readInt("So ghe: ", 1, 500);
    f.booked = 0;
    f.active = 1;

    flights[flightCount++] = f;
    saveData();
    msgOk("Them chuyen bay thanh cong.");
}

void listFlights(void) { 
    sectionTitle("DANH SACH CHUYEN BAY");
    int count = 0;
    flightHeader();
    for(int i = 0; i < flightCount; i++) {
        if(flights[i].active) {
            printFlight(&flights[i]);
            count++;
        }
    }
    if(count == 0) {
        printf("| " C_YELLOW "%-*s" C_RESET " |\n", W - 4, "Chua co chuyen bay nao.");
    }
    tableLine();
    printf("  Tong: " C_BOLD "%d" C_RESET " chuyen bay   (Ghe: " C_GREEN "con nhieu" C_RESET " / "
           C_YELLOW "sap day" C_RESET " / " C_RED "het cho" C_RESET ")\n", count);
}

// === CHUC NANG 3: TIM/LOC CHUYEN BAY ===
void searchFlights(void) {
    banner("TIM / LOC CHUYEN BAY");
    char keyword[50];
    readLine("Nhap noi di hoac noi den can tim: ", keyword, sizeof(keyword));
    
    sectionTitle("KET QUA TIM KIEM");
    int count = 0;
    flightHeader();
    for(int i = 0; i < flightCount; i++) {
        if(flights[i].active && (strcasecmp(flights[i].from, keyword) == 0 || strcasecmp(flights[i].to, keyword) == 0)) {
            printFlight(&flights[i]);
            count++;
        }
    }
    tableLine();
    if(count == 0) {
        msgInfo("Khong tim thay chuyen bay phu hop.");
    } else {
        printf("  Tim thay " C_BOLD "%d" C_RESET " chuyen bay.\n", count);
    }
}

// === CHUC NANG 6: HUY VE ===
void cancelTicket(void) {
    banner("HUY VE");
    char ticket[20];
    readLine("Nhap ma ve can huy: ", ticket, sizeof(ticket));
    
    int pIdx = findPassenger(ticket);
    if(pIdx < 0) {
        msgErr("Khong tim thay ma ve nay.");
        return;
    }
    
    Passenger *p = &passengers[pIdx];
    int fIdx = findFlight(p->flightCode);
    
    // Danh dau ve khong hoat dong
    p->active = 0;
    
    // Giam so luong ghe da dat cua chuyen bay tuong ung
    if(fIdx >= 0 && flights[fIdx].booked > 0) {
        flights[fIdx].booked--;
    }
    
    saveData();
    msgOk("Huy ve thanh cong.");
}

void listTickets(void) {
    banner("DANH SACH VE DA DAT");
    int count = 0;

    printf("+----------+----------+------------------------------+-----------------+\n");
    printf("|" C_BOLD C_CYAN " %-8s " C_RESET "|" C_BOLD C_CYAN " %-8s " C_RESET "|" C_BOLD C_CYAN " %-28s "
           C_RESET "|" C_BOLD C_CYAN " %-15s " C_RESET "|\n", "MA VE", "CHUYEN", "HO TEN", "DIEN THOAI");
    printf("+----------+----------+------------------------------+-----------------+\n");
    for(int i = 0; i < passengerCount; i++) {
        if(passengers[i].active) {
            printf("| " C_BOLD "%-8.8s" C_RESET " | %-8.8s | %-28.28s | %-15.15s |\n",
                   passengers[i].ticket,
                   passengers[i].flightCode,
                   passengers[i].fullName,
                   passengers[i].phone);
            count++;
        }
    }
    if(count == 0) {
        printf("| " C_YELLOW "%-69s" C_RESET " |\n", "Chua co ve nao duoc dat.");
    }
    printf("+----------+----------+------------------------------+-----------------+\n");
    printf("  Tong: " C_BOLD "%d" C_RESET " ve\n", count);
}

void saveData(void) { /* Luu file */ }

void searchTicket(void)
{
    banner("TRA CUU VE");
    char ticket[20];
    readLine("Nhap ma ve: ", ticket, sizeof(ticket));
    int idx = findPassenger(ticket);

    if(idx < 0)
    {
        msgErr("Khong tim thay ve.");
        return;
    }

    Passenger *p = &passengers[idx];
    int fIdx = findFlight(p->flightCode);
    printTicketCard(p, fIdx >= 0 ? &flights[fIdx] : NULL);
}

void updateFlight(void)
{
    banner("CAP NHAT CHUYEN BAY");
    char code[CODE_LEN];
    readLine("Nhap ma chuyen bay can sua: ", code, sizeof(code));
    int idx = findFlight(code);

    if(idx < 0)
    {
        msgErr("Khong tim thay.");
        return;
    }

    Flight *f = &flights[idx];
    sectionTitle("Thong tin hien tai");
    flightHeader();
    printFlight(f);
    tableLine();

    sectionTitle("Nhap thong tin moi");
    readLine("Noi di moi: ", f->from, sizeof(f->from));
    readLine("Noi den moi: ", f->to, sizeof(f->to));

    do
    {
        readLine("Ngay bay moi (YYYY-MM-DD): ", f->date, sizeof(f->date));
        if(!validDate(f->date))
            msgErr("Ngay khong hop le.");
    }
    while(!validDate(f->date));

    do
    {
        readLine("Gio bay moi (HH:MM): ", f->time, sizeof(f->time));
        if(!validTime(f->time))
            msgErr("Gio khong hop le.");
    }
    while(!validTime(f->time));

    f->price = readDouble("Gia ve moi (VND): ", 1000);
    f->capacity = readInt("So ghe moi: ", f->booked, 500);

    saveData();
    msgOk("Cap nhat thanh cong.");
}

void deleteFlight(void)
{
    banner("XOA CHUYEN BAY");
    char code[CODE_LEN];
    readLine("Nhap ma chuyen bay: ", code, sizeof(code));
    int idx = findFlight(code);

    if(idx < 0)
    {
        msgErr("Khong tim thay.");
        return;
    }

    flights[idx].active = 0;
    saveData();
    msgOk("Da xoa chuyen bay.");
}

void revenueReport(void)
{
    banner("THONG KE DOANH THU");
    double revenue = 0;
    int tickets = 0;
    for(int i = 0; i < passengerCount; i++)
    {
        if(!passengers[i].active)
            continue;

        int idx = findFlight(passengers[i].flightCode);
        if(idx >= 0)
        {
            revenue += flights[idx].price;
            tickets++;
        }
    }

    char money[32];
    fmtMoney(revenue, money, sizeof(money));

    printf("\n+--------------------------------------+\n");
    printf("| %-22s : %11d |\n", "So ve da ban", tickets);
    printf("| " C_BOLD "%-22s" C_RESET " : " C_BOLD C_GREEN "%11s" C_RESET " |\n", "Tong doanh thu (VND)", money);
    printf("+--------------------------------------+\n");
}

void bookTicket(void)
{
    banner("DAT VE MAY BAY");
    if(passengerCount >= MAX_PASSENGERS)
    {
        msgErr("Danh sach hanh khach da day.");
        return;
    }

    listFlights();
    printf("\n");

    char code[CODE_LEN];
    Passenger p = {0};

    readLine("Nhap ma chuyen bay: ", code, sizeof(code));
    int idx = findFlight(code);

    if(idx < 0)
    {
        msgErr("Khong ton tai chuyen bay.");
        return;
    }

    if(flights[idx].booked >= flights[idx].capacity)
    {
        msgErr("Da het cho.");
        return;
    }

    sectionTitle("Chuyen bay da chon");
    flightHeader();
    printFlight(&flights[idx]);
    tableLine();

    sectionTitle("Thong tin hanh khach");
    readLine("Ho ten: ", p.fullName, sizeof(p.fullName));
    if(strlen(p.fullName) < 3)
    {
        msgErr("Ten khong hop le.");
        return;
    }

    readLine("So dien thoai: ", p.phone, sizeof(p.phone));
    if(!validPhone(p.phone))
    {
        msgErr("So dien thoai khong hop le.");
        return;
    }

    snprintf(p.ticket, sizeof(p.ticket), "VE%06d", passengerCount + 1);
    strcpy(p.flightCode, code);
    p.active = 1;

    passengers[passengerCount++] = p;
    flights[idx].booked++;
    saveData();

    msgOk("Dat ve thanh cong!");
    printTicketCard(&p, &flights[idx]);
}

int compareFlight(const void *a, const void *b)
{
    const Flight *f1 = (const Flight*)a;
    const Flight *f2 = (const Flight*)b;

    int cmp = strcmp(f1->date, f2->date);
    if(cmp != 0)
        return cmp;

    return strcmp(f1->time, f2->time);
}

void sortFlights(void)
{
    banner("SAP XEP CHUYEN BAY");
    qsort(flights, flightCount, sizeof(Flight), compareFlight);
    msgOk("Da sap xep theo ngay, gio khoi hanh.");
    listFlights();
}

int main()
{
    int choice;

    enableAnsi();

    while(1)
    {
        clearScreen();

        mainMenu();

        choice = readInt("Lua chon: ",0,2);

        switch(choice)
        {
            case 1:
                if(adminLogin())
                    adminMenu();
                break;

            case 2:
                customerMenu();
                break;

            case 0:
                saveData();
                return 0;
        }
    }
}
