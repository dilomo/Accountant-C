#ifndef ACC_XML_WRITER_H_INCLUDED
#define ACC_XML_WRITER_H_INCLUDED

#include "Acc.Main.h"
#include "Acc.Xml.h"
#include "Acc.StringStack.h"

typedef struct xml_writer{
    FILE* xml_file;
    char  ident_char;          /*ident chars may be \t, space ot other chars*/
    int   identation;          /*the number of ident chars to insert*/
    int   depth;               /*the current depth of the writer*/
    bool  use_formatting;      /* weather to use the ident char and identation or not*/
    bool  open_node;           /* weather there is a node open */
    bool  has_value;           /* weather current node has a value */
    bool  open_brace;          /* weather there is opened brace*/
    bool  one_row;             /* the element spreads only in one row*/
    Stack opened_nodes;        /* containst all names of opened node*/
}XmlWriter;

bool InitXmlWriter(XmlWriter* writer, char * filename);
bool CloseXmlWriter(XmlWriter* writer);

/*writes the standar xml header <?xml version="1.0" ... */
bool WriteStartDoc(XmlWriter* writer);
/*Closes all opened tags if any*/
bool WriteEndDoc(XmlWriter* writer);

bool WriteStartElem(XmlWriter* writer,char * name);
/* === function that must be used after WriteStartElem and before writeEndElem */
bool WriteAttributeString(XmlWriter* writer, char * name, char * value);
bool WriteValueString(XmlWriter* writer, char * value);
bool WriteValueInt(XmlWriter* writer, int value);
/* === */
bool WriteEndElem(XmlWriter* writer);

bool WriteElementString(XmlWriter* writer, char * name, char * value);
bool WriteCommentString(XmlWriter* writer, char * comment);

/* arguments:   c   - the char to put before each new level (depth) of the doc   */
/*              num - number of times to repeat c                                */
/* if c is '\0' or num is 0 then formatting is disabled                          */
bool SetXmlWriterFormatting(XmlWriter* writer, char c, int num);

bool IsClosed(XmlWriter* writer);

#endif // ACC_XML_WRITER_H_INCLUDED
