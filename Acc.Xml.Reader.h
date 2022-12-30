/*=================================== Acc.Xml.Reader.h ========================================*/
#ifndef ACC_XML_READER_H_INCLUDED
#define ACC_XML_READER_H_INCLUDED

#include <ctype.h>
#include "Acc.Main.h"
#include "Acc.Xml.h"

typedef enum xml_node_type {
  /* Attribute, */
  /*  CDATA, */
    Document,
    Comment,
    Element,
    EndElement,
  /*  Text, */
    None
}NodeType;

typedef struct xml_string {
    char * name;
    char * attr_names[MAX_ATTR_COUNT];
    char * attr_values[MAX_ATTR_COUNT];
    char * value;
} XmlString;


typedef struct xml_reader {
    FILE *   xml_file;
    NodeType node_t;                /* the type of the node that is currently read in mem*/
//    char     nname[MAX_XML_STR];    /* the name of the read node (attribute, element ...)*/
//    char     nvalue[MAX_XML_STR];   /* the value of the current node*/
    char *   current_line;          /* the value of the current node*/
    char *   current_pos;           /* the position pinter*/
    int      attr_count;            /* used to traverse attributes*/
    bool     has_value;             /* true if the node has a value - other nodes ect.*/
    bool     has_string_value;      /* true if the node has a string value*/
//    int      depth;                 /* the deapth at which the node read is*/
    bool     IsEOF;                 /* true if the end of xml_file is reached or reader is Destroyed*/
    XmlString parsed_line;          /* This is spec struct that contains one xml elem separated into parts*/
}XmlReader;


/* operation:        initialize a XML list                                 */
/* preconditions:    reader points to a list and filename is existing file */
/* postconditions:   the file is opened and the reader is ready to Read    */
bool InitXmlReader(XmlReader* reader, char * filename);
bool CloseXmlReader(XmlReader* reader);

bool Read(XmlReader * reader);
NodeType GetNodeType(XmlReader * reader);

char * ReadElementString(XmlReader* reader);
char * ReadElementName(XmlReader *reader);

int    AttributeCount(XmlReader *reader);
char * GetAttribureName(XmlReader* reader, int index);
char * GetAttribureValue(XmlReader *reader, int index);

bool IsEOF(XmlReader* reader);
bool HasValue(XmlReader* reader);
bool HasSubNodes(XmlReader* reader);

#endif // ACC_XML_READER_H_INCLUDED
