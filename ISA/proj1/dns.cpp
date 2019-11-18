/**
    dns.cpp
    Purpose: DNS resolver
    @author Alex Sporni
    Email: xsporn01@stud.fit.vutbr.cz
    Version: 1.0 | 18.11.2019  
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

using namespace std;

/**
    Prototypy funkcii 
*/
unsigned char* ReadName (unsigned char*,unsigned char*,int*);
void ChangetoDnsNameFormat (unsigned char*,unsigned char*);
void dns_resolve(const char *server_name, char *host);
void parseInputArguments(int argc, char **argv);
void print_help();
void reverse_adress(char *requested_server);
void print_sections(struct RES_RECORD *array, int *i);
string ipv6_to_str_unexpanded(const struct in6_addr * addr);
bool is_port_number(const string& s);

/**
    Globalne premenne pre pracu s prepinacmi a vstupmi
*/
bool r_input = false;
bool x_input = false;
bool ipv6_input = false;
bool s_input = false;
bool p_input = false;

string port_number;
string ip_address;
string domain_name;

char * requested_server;
char ipv4_ptr_record[INET_ADDRSTRLEN + 13];
char fake_qname[INET6_ADDRSTRLEN];

int int_port_number;
int dns_server_count = 0;
int r_recursionDesired = 0;
int x_reversionDesired = 0;
int ipv6Desired = 0;
int query_type = 1;

#define default_dns_port_number 53

/**
    Typy DNS tried
    http://edgedirector.com/app/type.htm
*/
#define CLASS_IN 1
#define CLASS_CSNET 2
#define CLASS_CHAOS 3
#define CLASS_HESIOD 4

/**
    Typy DNS zaznamov 
    https://en.wikipedia.org/wiki/List_of_DNS_record_types
*/
#define TYPE_A 1 //Returns a 32-bit IPv4 address, most commonly used to map hostnames to an IP address of the host
#define TYPE_NS 2 //Delegates a DNS zone to use the given authoritative name servers
#define TYPE_MD 3 //Mail destination
#define TYPE_MF 4 //Mail forwarder record
#define TYPE_CNAME 5 // Alias of one name to another: the DNS lookup will continue by retrying the lookup with the new name
#define TYPE_SOA 6 // Specifies authoritative information about a DNS zone including the primary name server etc...
//MB, MG, MR, and MINFO are records to publish subscriber mailing lists.
#define TYPE_MB 7 
#define TYPE_MG 8
#define TYPE_MR 9
#define TYPE_NULL 10 //Obsoleted by RFC 1035. RFC 883 defined "completion queries" which used this record.
#define TYPE_WKS 11 //Record to describe well-known services supported by a host.
#define TYPE_PTR 12 // domain name pointer
#define TYPE_HINFO 13 //Record intended to provide information about host CPU type and operating system
#define TYPE_MINFO 14 
#define TYPE_MX 15 //Mail server
#define TYPE_TXT 16 //Originally for arbitrary human-readable text in a DNS record.
#define TYPE_AAAA 28 //Returns a 128-bit IPv6 address, most commonly used to map hostnames to an IP address of the host.


/**
    
    Poznamka k implementacii: Pri rieseni daneho problemu mi pomohlo podobne riesenie zostavenia DNS struktury: 
    https://gist.github.com/fffaraz/9d9170b57791c28ccda9255b48315168
    autor: Silver Moon (m00n.silv3r@gmail.com)

*/
//DNS hlavicka v little endian formate
struct DNS_HEADER
{
    unsigned short id; // identification number
 
    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag
 
    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available
 
    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};

/**
    DNS otazka
*/
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};
 
#pragma pack(push, 1)
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)
 

struct RES_RECORD
{
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
};
 
/**
    Struktura DNS query
*/
typedef struct
{
    unsigned char *name;
    struct QUESTION *ques;
} QUERY;

/**
    Funkcia pre zavolanie pomocnej napovedy, pomocna napoveda za zavola v pripade
    zadania nepodporovaneho prepinaca
    @return pomocna napoveda
*/
void print_help()
{
    cout <<
            "Povolene prikazy:\n"
            "-r:        Požadována rekurze (Recursion Desired = 1), jinak bez rekurze.\n"
            "-x:        Reverzní dotaz místo přímého.\n"
            "-6:        Dotaz typu AAAA místo výchozího A.\n"
            "-s:        IP adresa nebo doménové jméno serveru, kam se má zaslat dotaz.\n"
            "-p:        port: Číslo portu, na který se má poslat dotaz, výchozí 53.\n";
    exit(EXIT_FAILURE);
}

void print_sections(struct RES_RECORD *array, int *i)
{
    //DNS zaznamy
    printf(" %s, ", array[*i].name);

    if (ntohs(array[*i].resource->type) == TYPE_A)
    {
        printf("A, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_NS)
    {
        printf("NS, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_MD)
    {
        printf("MD, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_MF)
    {
        printf("MF, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_CNAME)
    {
        printf("CNAME, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_SOA)
    {
        printf("SOA, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_MB)
    {
        printf("MB, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_MG)
    {
        printf("MG, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_MR)
    {
        printf("MR, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_NULL)
    {
        printf("NULL, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_WKS)
    {
        printf("MKS, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_PTR)
    {
        printf("PTR, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_HINFO)
    {
        printf("HINFO, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_MINFO)
    {
        printf("MINFO, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_MX)
    {
        printf("MX, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_TXT)
    {
        printf("TXT, ");
    }
    if (ntohs(array[*i].resource->type) == TYPE_AAAA)
    {
        printf("AAAA, ");
    }
    
    //DNS triedy
    if (ntohs(array[*i].resource->_class) == CLASS_IN)
    {
        printf("IN, ");
    }
    if (ntohs(array[*i].resource->_class) == CLASS_CSNET)
    {
        printf("CSNET, ");
    }
    if (ntohs(array[*i].resource->_class) == CLASS_CHAOS)
    {
        printf("CHAOS, ");
    }
    if (ntohs(array[*i].resource->_class) == CLASS_HESIOD)
    {
        printf("HESOID, ");
    }

    //Time To Live
    printf("%d, ", ntohl(array[*i].resource->ttl)); 
}

/**
    Funkcia, ktora sluzi na otocenie IPv4 adresy a pridanie pripony "in-addr.arpa",
    z formatu 104.20.0.85 na 85.0.20.104.in-addr.arpa

    @param  IPv4 adresa vo formate 104.20.0.85
    @return IPv4 adresa vo formate 85.0.20.104.in-addr.arpa
    Poznamka k implementacii: Pri rieseni daneho problemu mi pomohlo vlakno: 
    https://stackoverflow.com/questions/16373248/convert-ip-for-reverse-ip-lookup/16373300#16373300
    autor odpovede: https://stackoverflow.com/users/440558/some-programmer-dude

*/
void reverse_adress(char *requested_server)
{
    char reversed_ip[INET_ADDRSTRLEN];
    in_addr_t addr;
    //konverzia do binarneho formatu
    inet_pton(AF_INET, requested_server, &addr);
    addr =
    ((addr & 0xff000000) >> 24) |
    ((addr & 0x00ff0000) >>  8) |
    ((addr & 0x0000ff00) <<  8) |
    ((addr & 0x000000ff) << 24);

    //Otocena IPv4 adresa sa nachadza v premennej reversed_ip
    //Prevedenie spat z binarneho formatu
    inet_ntop(AF_INET, &addr, reversed_ip, sizeof(reversed_ip));
    
    //Pridanie pripony ".in-addr.arpa"
    char concatenated_address[INET_ADDRSTRLEN + 13];
    strcpy(concatenated_address, reversed_ip);
    strcat(concatenated_address, ".in-addr.arpa");
    strcpy(ipv4_ptr_record, concatenated_address);  
}

/**
    Funkcia, ktora sluzi na rozsirenie IPv6 adresy napr. 2001:67c:1220:809::93e5:91a --> 2001:067C:1220:0809:0000:0000:93E5:091A
    dalej funkcia podporuje otocenie IPv6 adresy, nahradzuje ':' za '.' a prida priponu "ip6.arpa"

    @param IPv6 adresa dotazovaneho servera
    @return IPv6 adresa v korektnom formate napr. (a.1.9.0.5.e.3.9.0.0.0.0.0.0.0.0.9.0.8.0.0.2.2.1.c.7.6.0.1.0.0.2.ip6.arpa.)
    Poznamka k implementacii: Pri rieseni daneho problemu mi pomohlo vlakno:
    https://stackoverflow.com/questions/3727421/expand-an-ipv6-address-so-i-can-print-it-to-stdout?fbclid=IwAR1I0JqZPGcKAMbAonZBlRZagX4_CQzCc1LjxH56I97Qgn7seL-cMGtnH3Q
    autor odpovede: https://stackoverflow.com/users/299301/nategoose

*/
string ipv6_to_str_unexpanded(char *dns_server) 
{
    char *expanded_ipv6 = (char*)malloc(100);
    struct in6_addr sin6_addr;
    if(inet_pton(AF_INET6, (dns_server), &(sin6_addr)) == 1) 
    {
        sprintf(expanded_ipv6, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                 (int)sin6_addr.s6_addr[0], (int)sin6_addr.s6_addr[1],
                 (int)sin6_addr.s6_addr[2], (int)sin6_addr.s6_addr[3],
                 (int)sin6_addr.s6_addr[4], (int)sin6_addr.s6_addr[5],
                 (int)sin6_addr.s6_addr[6], (int)sin6_addr.s6_addr[7],
                 (int)sin6_addr.s6_addr[8], (int)sin6_addr.s6_addr[9],
                 (int)sin6_addr.s6_addr[10], (int)sin6_addr.s6_addr[11],
                 (int)sin6_addr.s6_addr[12], (int)sin6_addr.s6_addr[13],
                 (int)sin6_addr.s6_addr[14], (int)sin6_addr.s6_addr[15]);
    }
    
    string server = string{expanded_ipv6};
    string inversed_address="";

    int i = server.length() - 1;
    char c;
    while(i >= 0) {
        c = server[i];
        if(c == ':') {
            i--;
            continue;
        }
        inversed_address += c; 
        inversed_address += '.'; 
        i--;
    }
    inversed_address += "ip6.arpa";
    free(expanded_ipv6);
    return inversed_address;
    
}

/**
    Funkcia, ktora sluzi na konvertovanie formatu
    z www.google.com na 3www6google3com 
    @param ukazatel na zaciatok dns query 
    @param dotazovana adresa
    @return korektny DNS name format

    Poznamka k implementacii: Pri rieseni problemu mi pomohlo podobne riesenie problematiky https://gist.github.com/fffaraz/9d9170b57791c28ccda9255b48315168
    riadok kodu: 399
    Autor: Silver Moon (m00n.silv3r@gmail.com)
    
*/
void ChangetoDnsNameFormat(unsigned char* dns, char *host) 
{
    unsigned int lock = 0 , i;
    strcat((char*)host,".");
     
    for(i = 0 ; i < strlen((char*)host) ; i++) 
    {
        if(host[i]=='.') 
        {
            *dns++ = i-lock;
            for(;lock<i;lock++) 
            {
                *dns++=host[lock];
            }
            lock++; //or lock=i+1;
        }
    }
    *dns++='\0';
}

/**
    Funkcia, ktora sluzi na prelozenie domenoveho mena pozadovaneho servera na IP adresu,
    vytvorenie, odoslanie paketu a nasledne prijatie odpovede od servera. 
    Vypisanie pozadovanych informacii na standardny vystup
    @param  IP adresa alebo domenove meno servera, kam sa ma zaslat dotaz napr. kazi.fit.vutbr.cz
    @param dotazovana adresa vo formate domenoveho mena napr. www.fit.vut.cz,
                                                        alebo IPv4 adresy (podpora PTR zaznamu) mozny vstup: ./dns -r -x -s kazi.fit.vutbr.cz 147.229.9.26
                                                        alebo IPv6 adresy (podpora PTR zaznamu pre IPv6) mozny vstup: ./dns -r -s kazi.fit.vutbr.cz 2001:67c:1220:809::93e5:91a -6 -x
                                                        alebo dotaz typu AAAA, mozny vstup: ./dns -r -s kazi.fit.vutbr.cz www.ietf.org -6
    @return Na standardny vystup vypise informacie o tom ci bola ziskana odpoved autoritativna, ci bola pouzita rekurzia
    alebo ci bola odpoved skratena. Dalej vypise vsetky sekcie a zaznamy v obdrzanej odpovedi, pre kazdu sekciu v odpovedi vypise jej nazov a pocet ziskanych zaznamov
    pre kazdy zaznam vypise jeho nazov, typ, triedu, hodnotu TTL a data.
    Poznamka k implementacii: Pri rieseni daneho problemu mi pomohol manual:   https://linux.die.net/man/3/getaddrinfo
*/
void dns_resolve(const char *server_name, char *host)
{   
    struct addrinfo hints, *servinfo;
    struct timeval tv;
    tv.tv_sec = 2;

    int try_socket;
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_UNSPEC;    // AF_UNSPEC podporuje IPv4 aj IPv6
    hints.ai_socktype = SOCK_DGRAM; // Jedna sa o datagram socket
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    if ((try_socket = getaddrinfo(server_name, NULL, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(try_socket));
        exit(EXIT_FAILURE);
    }

    unsigned char buf[65536],*qname,*reader;
    int i , j , stop , s;
    struct sockaddr_in a;
    struct RES_RECORD answers[20],auth[20],addit[20]; //Odpovede od DNS servera
    struct sockaddr_in *dest;
    struct DNS_HEADER *dns = NULL;
    struct QUESTION *qinfo = NULL;
 
    s = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP); //UDP paket na DNS dotazy
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) 
    {
        perror("ERROR!");
        exit(EXIT_FAILURE);
    }
 
    dest = (struct sockaddr_in *)servinfo->ai_addr;
    dest->sin_port = htons(int_port_number);
 
    //Nastavi DNS strukturu
    dns = (struct DNS_HEADER *)&buf;
 
    dns->id = (unsigned short) htons(getpid());
    dns->qr = 0;
    dns->opcode = 0; 
    dns->aa = 0; 
    dns->tc = 0; 
    dns->rd = r_recursionDesired; //V pripade pozadovanej rekurzie sa flag nastavi na 1
    dns->ra = 0; 
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1);
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;

     //ukazatel na query oblast
    qname =(unsigned char*)&buf[sizeof(struct DNS_HEADER)];
    /*
        Ak je zvoleny parameter -6 tak sa jedna o AAAA dotaz, query_type sa nastavi na TYPE_AAAA (int 28)
    */
    if(ipv6Desired)
    {
        query_type = TYPE_AAAA;
    }
    /*
        Ak je zvoleny parameter -6 a parameter -x tak sa jedna o reverzny IPv6 dotaz, query_type sa nastavi na TYPE_PTR (int 12)
    */
    if (x_reversionDesired && ipv6Desired)
    {
        query_type = TYPE_PTR;
        string reversedIPv6 = ipv6_to_str_unexpanded(requested_server);
        strcpy(fake_qname, (char *)reversedIPv6.c_str());
        ChangetoDnsNameFormat(qname , fake_qname);
    } 
    else if (x_reversionDesired) 
    {
        /*
            Ak je pozadovany reverzny zaznam tak nastav query_type na hodnotu TYPE_PTR (int 12)
            Otoc pozadovanu adresu na format 85.1.20.104.in-addr.arpa.
            Zavolaj funkciu na konverziu formatu  
        */
        query_type = TYPE_PTR;
        reverse_adress(host);
        ChangetoDnsNameFormat(qname , ipv4_ptr_record);
    }
    else
    {
        ChangetoDnsNameFormat(qname , host);
    }

    qinfo =(struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)]; //ziskanie informacii
    
    qinfo->qtype = htons(query_type); //typ DNS query: A , AAAA , CNAME, PTR...
    qinfo->qclass = htons(1);
 
    //Posielanie paketu
    if( sendto(s,(char*)buf,sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION),0,(struct sockaddr*)dest,sizeof(*dest)) < 0)
    {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(servinfo); // uvolnenie dynamicky alokovanej pamate
    //Prijatie odpovede
    i = sizeof dest;
    if(recvfrom (s,(char*)buf , 65536 , 0 , (struct sockaddr*)&dest , (socklen_t*)&i ) < 0)
    {
        perror("recvfrom failed");
        exit(EXIT_FAILURE);
        
    }
    
    dns = (struct DNS_HEADER*) buf;
 
    //posuniem sa v zasobniku, preskocim header a query, chcem citat DNS odpoved
    reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];

    if (dns->aa > 0)
    {
        printf("Authoritative: YES, ");
    }
    else
    {
        printf("Authoritative: NO, ");
    }
    if (dns->rd > 0)
    {
        printf("Recursive: YES, ");
    }
    else
    {
        printf("Recursive: NO, ");
    }
    if (dns->tc > 0)
    {
        printf("Truncated: YES\n");
    }
    else
    {
        printf("Truncated: NO\n");
    }
    if (dns->rcode != 0)
    {
        fprintf(stderr, "Error, server reply code: (%d), \n", dns->rcode);
        if (dns->rcode == 1)
        {
            printf("Format error - The name server was unable to interpret the query.\n");
        }
        else if (dns->rcode == 2)
        {
            printf("Server failure - The name server was unable to process this query due to a problem with the name server.\n");
        }
        else if (dns->rcode == 3)
        {
            printf("Name Error - This code signifies that the domain name referenced in the query does not exist.\n");
        }
        else if (dns->rcode == 4)
        {
            printf("Not Implemented - The name server does not support the requested kind of query.\n");
        }
        else if (dns->rcode == 5)
        {
            printf("Refused - The name server refuses to perform the specified operation for policy reasons.\n");
        }
        else
        {
            printf("Error, unspecified server response\n");
        }
        
        exit(EXIT_FAILURE);
    }
    
    //Zaciatok citania odpovede
    stop=0;
 
    for(i=0; i<ntohs(dns->ans_count); i++)
    {
        answers[i].name=ReadName(reader,buf,&stop);
        reader = reader + stop;
 
        answers[i].resource = (struct R_DATA*)(reader);
        reader = reader + sizeof(struct R_DATA);
 
        if(ntohs(answers[i].resource->type) == TYPE_A)
        {
            answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len));
 
            for(j=0 ; j<ntohs(answers[i].resource->data_len) ; j++)
            {
                answers[i].rdata[j]=reader[j];
            }
 
            answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';
 
            reader = reader + ntohs(answers[i].resource->data_len);
        }
        else if(ntohs(answers[i].resource->type) == TYPE_AAAA)
        {
            answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len));
 
            for(j=0 ; j<ntohs(answers[i].resource->data_len) ; j++)
            {
                answers[i].rdata[j]=reader[j];
            }
 
            answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';
 
            reader = reader + ntohs(answers[i].resource->data_len);
        }
        else
        {
            answers[i].rdata = ReadName(reader,buf,&stop);
            reader = reader + stop;
        }
    }
    //citanie Authority section
    for(i=0; i<ntohs(dns->auth_count); i++)
    {
        auth[i].name=ReadName(reader,buf,&stop);
        reader+=stop;
 
        auth[i].resource=(struct R_DATA*)(reader);
        reader+=sizeof(struct R_DATA);
 
        auth[i].rdata=ReadName(reader,buf,&stop);
        reader+=stop;
    }
 
    //citanie Additional section
    for(i=0; i<ntohs(dns->add_count); i++)
    {
        addit[i].name=ReadName(reader,buf,&stop);
        reader+=stop;
 
        addit[i].resource=(struct R_DATA*)(reader);
        reader+=sizeof(struct R_DATA);
 
        if(ntohs(addit[i].resource->type) == TYPE_A)
        {
            addit[i].rdata = (unsigned char*)malloc(ntohs(addit[i].resource->data_len));
            for(j=0;j<ntohs(addit[i].resource->data_len);j++)
            addit[i].rdata[j]=reader[j];
 
            addit[i].rdata[ntohs(addit[i].resource->data_len)]='\0';
            reader+=ntohs(addit[i].resource->data_len);
        }
        else if(ntohs(addit[i].resource->type) == TYPE_AAAA)
        {
            addit[i].rdata = (unsigned char*)malloc(ntohs(addit[i].resource->data_len));
            for(j=0;j<ntohs(addit[i].resource->data_len);j++)
            addit[i].rdata[j]=reader[j];
 
            addit[i].rdata[ntohs(addit[i].resource->data_len)]='\0';
            reader+=ntohs(addit[i].resource->data_len);
        }
        else
        {
            addit[i].rdata=ReadName(reader,buf,&stop);
            reader+=stop;
        }
    }
    //Vystup question sekcie
    printf("Question section (%d)\n", ntohs(dns->q_count));
    if (x_reversionDesired && ipv6Desired)
    {
         printf("%s ", fake_qname);
    }
    if (x_reversionDesired)
    {
         printf("%s, ", ipv4_ptr_record);
    }
    else
    {
       printf("%s, ", requested_server);
    }  
    if (ntohs(qinfo->qtype) == TYPE_A)
    {
        printf("A, ");
    }
    else if (ntohs(qinfo->qtype) == TYPE_CNAME)
    {
        printf("CNAME, ");
    }
    else if (ntohs(qinfo->qtype) == TYPE_PTR)
    {
        printf("PTR, ");
    }
    else if (ntohs(qinfo->qtype) == TYPE_AAAA)
    {
        printf("AAAA, ");
    }
    if (ntohs(qinfo->qclass) == CLASS_IN)
    {
        printf("IN\n");
    }

    //Vystup answer section
    printf("\nAnswer section (%d) \n" , ntohs(dns->ans_count) );
    for(i=0 ; i < ntohs(dns->ans_count) ; i++)
    {
        print_sections(answers, &i);
        if( ntohs(answers[i].resource->type) == TYPE_A)
        {
            long *p;
            p=(long*)answers[i].rdata;
            a.sin_addr.s_addr=(*p);
            printf("%s",inet_ntoa(a.sin_addr));
        }
        if(ntohs(answers[i].resource->type) == TYPE_CNAME)
        {
            //CNAME
            printf("%s",answers[i].rdata);
        }
        if(ntohs(answers[i].resource->type) == TYPE_PTR) 
        {
            //PTR
            printf("%s",answers[i].rdata);
        }
        if(ntohs(answers[i].resource->type) == TYPE_AAAA) 
        {
            //AAAA
            void *p;
            p=answers[i].rdata;
            char buffer[INET6_ADDRSTRLEN];
            char adresa[16];
            memcpy(adresa, p, 16);
            const char* result = inet_ntop(AF_INET6, p, buffer, INET6_ADDRSTRLEN);
            cout << result;
        }
        printf("\n");
    }
    //Vystup Authority section
    printf("Authority section (%d)\n" , ntohs(dns->auth_count) );
    for( i=0 ; i < ntohs(dns->auth_count) ; i++)
    {
        print_sections(auth, &i);
        if(ntohs(auth[i].resource->type) == TYPE_NS)
        {
            printf("%s.",auth[i].rdata);
        }
        printf("\n");
    }
 
    //Vystup Additional section
    printf("\nAdditional section (%d) \n" , ntohs(dns->add_count) );
    for(i=0; i < ntohs(dns->add_count) ; i++)
    {
        //printf("Name : %s ",addit[i].name);
        print_sections(addit, &i);
        if(ntohs(addit[i].resource->type) == TYPE_A)
        {
            long *p;
            p=(long*)addit[i].rdata;
            a.sin_addr.s_addr=(*p);
            printf("%s",inet_ntoa(a.sin_addr));
        }
        if(ntohs(addit[i].resource->type) == TYPE_AAAA)
        {
            void *p;
            p=addit[i].rdata;
            char buffer[INET6_ADDRSTRLEN];
            char adresa[16];
            memcpy(adresa, p, 16);
            const char* result = inet_ntop(AF_INET6, p, buffer, INET6_ADDRSTRLEN);
            cout << result;
        }
        printf("\n");
    }
    return;
}

/**
    Funkcia, ktora sluzi na citanie odpovede
    Poznamka k implementacii: Pri rieseni problemu mi pomohlo podobne riesenie problematiky https://gist.github.com/fffaraz/9d9170b57791c28ccda9255b48315168
    Autor: Silver Moon (m00n.silv3r@gmail.com)

    @param obsah zasobnika reader s odpovedou
    @param zasobnik
    @param pozicia v zasobniku
    @return domenove meno

*/
u_char* ReadName(unsigned char* reader, unsigned char* buffer, int* count)
{
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;
 
    *count = 1;
    name = (unsigned char*)malloc(256);
 
    name[0]='\0';
 
    //precita meno v formate 3www6google3com 
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152;
            reader = buffer + offset - 1;
            jumped = 1;
        }
        else
        {
            name[p++]=*reader;
        }
 
        reader = reader+1;
 
        if(jumped==0)
        {
            *count = *count + 1;
        }
    }
    name[p]='\0';
    if(jumped==1)
    {
        *count = *count + 1; //pocet krokov o ktore sme sa pohli pri citani v pakete
    }
    //skonvertuje 3www6google3com0 to www.google.com
    for(i=0;i<(int)strlen((const char*)name);i++) 
    {
        p=name[i];
        for(j=0;j<(int)p;j++) 
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    return name;
}

/**
    Funkcia, ktora sluzi na validovanie portoveho cisla, kontroluje 
    ci zadany port ma spravny datovy typ v nasom pripade int
    @param  cislo portu
    @return vysledok testu ci sa jedna o true alebo false 
*/
bool is_port_number(const string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

/**
    Funkcia, ktora sluzi na rozparsovanie vstupnych argumentov,
    vyuziva getops, musi byt zadany minimalne prepinac -s a dotazovana adresa.
    Vsetky prepinace mozu byt zadane najviac jedenkrat.
    Funkcia dalej vola pomocne funkcie pre pracu s argumentmi 

    @param  pocet argumentov
    @param  vstupne argumenty
    Poznamka k implementacii: Pri rieseni daneho problemu mi pomohol odkaz https://pubs.opengroup.org/onlinepubs/7908799/xsh/getopt.html

*/
void parseInputArguments(int argc, char **argv)
{
    int option_index = 0;
    while ((option_index = getopt(argc, argv, ":rx6s:p:")) != -1)
    {
        switch (option_index)
        {
            case 'r':
            if(r_input)
            {
                printf("ERROR, argumet -r bol uz pouzity !\n");
                exit(EXIT_FAILURE);
            }
            r_recursionDesired = 1;
            r_input = true;
            break;

            case 'x':
            if(x_input)
            {
                printf("ERROR, argument -x bol uz pouzity !\n");
                exit(EXIT_FAILURE);
            }
            x_reversionDesired = 1;
            x_input = true;
            break;

            case '6':
            if(ipv6_input)
            {
                printf("ERROR, argument -6 bol uz pouzity !\n");
                exit(EXIT_FAILURE);
            }
            ipv6Desired = 1;
            ipv6_input = true;
            break;

            case 's':
            if(s_input)
            {
                printf("ERROR, argument -s bol uz pouzity !\n");
                exit(EXIT_FAILURE);
            }
            ip_address = optarg;
            s_input = true;
            break;

            case 'p':
            if(p_input)
            {
                printf("ERROR, argument -p bol uz pouzity !\n");
                exit(EXIT_FAILURE);
            }
            p_input = true;
            port_number = optarg;
            if (is_port_number(port_number))
            {
                int_port_number = atoi(port_number.c_str());
                if ((int_port_number < 1) || (int_port_number > 65535))
                {
                    printf("ERROR, cislo portu nie je z povoleneho rozsahu <1,65535>\n");
                    exit(EXIT_FAILURE); 
                }            
            }
            else
            {
                printf("ERROR, cislo portu musi byt validny integer\n");
                exit(EXIT_FAILURE);
            }
            break;

            case '?':
            printf("ERROR, invalid option: %c\n", char(optopt));
            print_help();
            break;

            case ':':
            printf("ERROR, missing argument for: %c\n", char(optopt));
            print_help();
            break;
        }
    }
    if (argc < 2)
    {
        printf("ERROR, Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }
    if (argc < (optind+1)) 
    {
        printf("ERROR, too few arguments\n");
        exit(EXIT_FAILURE);
    }
    if (argc > (optind+1))
    {
        printf("ERROR, too many arguments\n");
        exit(EXIT_FAILURE);
    }
    if (argc == (optind)+1)
    {
        requested_server =  argv[optind];
    }
    if (p_input == false)
    {
        int_port_number = default_dns_port_number;
    }
}

int main(int argc, char **argv)
{
    parseInputArguments(argc, argv);
    dns_resolve(ip_address.c_str(), requested_server);

    return 0;
}