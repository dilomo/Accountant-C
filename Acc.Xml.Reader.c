/*=================================== Acc.Xml.Reader.c ========================================*/
#include "Acc.Xml.Reader.h"

static bool parsexml(XmlReader * reader);
static bool setXmlString(XmlReader * reader);

bool InitXmlReader(XmlReader * reader, char * filename)
{
    FILE * file = fopen(filename, "r");
    if (file == NULL)
        return false;

    /*initialize all properties */
    reader->xml_file = file;
    reader->node_t = None;
//    clearstrn(reader->nname, MAX_XML_STR);
//    clearstrn(reader->nvalue, MAX_XML_STR);

    /*create dyn string*/
    reader->current_line = (char * ) malloc(MAX_XML_STR * sizeof(char));
    if (reader->current_line == NULL)
        return false;

    reader->current_pos = reader->current_line;


    clearstrn(reader->current_line, MAX_XML_STR);
    reader->has_value = false;
    reader->has_string_value = false;
    reader->attr_count = 0;
    reader->IsEOF = false;
    /*init XmlString*/
    reader->parsed_line.name = NULL;
    int i;
    for (i = 0; i < MAX_ATTR_COUNT; ++i) {
        reader->parsed_line.attr_names[i] = NULL;
        reader->parsed_line.attr_values[i] = NULL;
    }
    reader->parsed_line.value = NULL;

    return true;
}

bool CloseXmlReader(XmlReader* reader)
{
    reader->node_t = None;
//    clearstr(reader->nname);
//    clearstr(reader->nvalue);
    free(reader->current_line);
    reader->has_value = false;
    reader->has_string_value = false;
    reader->attr_count = 0;
    reader->IsEOF = true; /*end of file is reached*/
    reader->current_pos = NULL;
    reader->parsed_line.name = NULL;

    int i;
    for (i = 0; i < MAX_ATTR_COUNT; ++i) {
        reader->parsed_line.attr_names[i] = NULL;
        reader->parsed_line.attr_values[i] = NULL;
    }
    reader->parsed_line.value = NULL;

    if (reader->xml_file != NULL)
        return (!fclose(reader->xml_file));
    else return true;
}

bool Read(XmlReader *reader)
{
    FILE * fin = reader->xml_file;
    if (fin == NULL) return false;
    if (reader->IsEOF == true) return false;

    /*init i to zero and clear string used to hold current line*/
    char c; int i = 0;
    clearstr(reader->current_line);

    while ((c = getc(fin)) != '\n' && c != EOF && c != '\0') {
        if ( i < MAX_XML_STR )
            reader->current_line[i] = c;
        i++;
    }
    /*set copy of current_line to be changed later in parsexml()*/
    reader->current_pos = reader->current_line;

    if ( c == EOF || c == '\0' ) {
        reader->IsEOF = true;
        //return;
    }

    /*determine what the row contains: element, header, comment ... */
    /*set up the XmlString fields accordingly*/
    return parsexml(reader);
}

NodeType GetNodeType(XmlReader * reader)
{
    return (reader->node_t);
}

char * ReadElementString(XmlReader *reader)
{
    return (reader->parsed_line.value);
}

char * ReadElementName(XmlReader *reader)
{
    return (reader->parsed_line.name);
}

int AttributeCount(XmlReader *reader)
{
    return reader->attr_count;
}

char * GetAttribureName(XmlReader *reader, int index)
{
    if (index >= MAX_ATTR_COUNT)
        return NULL;

    return (reader->parsed_line.attr_names[index]);
}

char * GetAttribureValue(XmlReader *reader, int index)
{
    if (index >= MAX_ATTR_COUNT)
        return NULL;

    return (reader->parsed_line.attr_values[index]);
}

bool IsEOF(XmlReader* reader)
{
    if (reader->IsEOF)
        return true;
    else
        return false;
}

bool HasValue(XmlReader* reader)
{
    return (reader->has_string_value);

}

bool HasSubNodes(XmlReader* reader)
{
    return (reader->has_value);
}


/* Private Functions */

static bool parsexml(XmlReader * reader)
{
    /*start index: different for each node type*/
    int index = 0;

    if (reader->current_pos == NULL)
        reader->current_pos =reader->current_line;

    /* we use reader->current_pos because reader->current_line should not be altered
        in order free() to work properly. Before the call to the function reader->current_pos
        should be initialized with the value of reader->current_line. This is done in the Read()
        function bu should be done manually if you use it for other purpose
    */
    /*trim start with padding char*/
    if (reader->current_pos[0] != '<')
        trim_start(reader->current_pos[0], reader->current_pos);

    /*== Set up the line type ==*/

    /* Normal element*/
    reader->node_t = Element;

    /* Comment*/
    if (strstr(reader->current_pos, "<!-- ") != NULL) {
        reader->node_t = Comment;

        if (strstr(reader->current_pos, " -->") != NULL)
            reader->has_value = false;

        index = 4;

        /*strange but should be called here too*/
        if (!setXmlString(reader))
            return false;
    }
    /* Document header */
    if (strstr(reader->current_pos, "<?") != NULL) {
        reader->node_t = Document;
        index = 2;
    }

    /* End Element - it has no attributes and value*/
    if (reader->current_pos[0] == '<' && reader->current_pos[1] == '/') {
        reader->node_t = EndElement;
        index = 2;
    }

    if (reader->node_t != EndElement && reader->IsEOF)
        return false;

    /* == Get attr count and check for value ==*/

    /*check for end tags element */
    if (strstr(reader->current_pos, "</") != NULL )
        reader->has_value = false;
    else {
        reader->has_value = true;
    }

    /* Find num of attributes if any ...*/
    reader->attr_count = 0;
    while (reader->current_pos[index] != '>') {
        if (reader->current_pos[index] == '=')
            reader->attr_count ++;
        index++;
    }
    if (reader->attr_count > MAX_ATTR_COUNT) {
        printe("Xml attributes count trimmed from %d to %s", reader->attr_count, MAX_ATTR_COUNT);
        reader->attr_count = MAX_ATTR_COUNT;
    }
//    printf("(%d,", reader->attr_count );

    /*check if the node has a value */

    reader->has_string_value = false;
    /*  index+1 should point to '^'
     if not end of line e.g. <name>value</name> or <name></name>
                                   ^
     else :                  <name/>
                                    ^
    */
    if (reader->current_pos[index+1] != '\0' && reader->current_pos[index+1] != '<' &&
     (reader->current_pos[index+1] != '/' && reader->current_pos[index+2] != '>') && reader->has_value == false) {
            reader->has_string_value = true;
    }

//    printf("%c) ", reader->has_string_value == true ? 't' : 'f');

    /*parse it according to the type*/
    if (!setXmlString(reader))
        return false;

    return true;

}

static bool setXmlString(XmlReader * reader)
{
    char * line = reader->current_pos;
    int attr_i = 0;
    if (line == NULL )
        return false;

    switch (reader->node_t) {
        case Element: /*Element with and w/o attributes*/
            while( *(line) == '<')
                line++;

            reader->parsed_line.name = line;

            while( *(line) != ' ' && *(line) != '/' && *(line) != '>' )
               line++;

            *(line) = '\0';
            line++;

            /*set attributes*/
            if (reader->attr_count > 0) {
                for (attr_i = 0 ; attr_i < reader->attr_count; attr_i++) {

                    /*skip spaces*/
                    while( *(line) == ' ')
                        line++;

                    reader->parsed_line.attr_names[attr_i] = line;

                    while( *(line) != '=')
                        line++;

                    *(line) = '\0';
                    line++;
                    if (*(line) == '"')  line++; /* skip quotes*/

                    reader->parsed_line.attr_values[attr_i] = line;

                    while( *(line) != '"')
                        line++;

                    *(line) = '\0';
                }
            }

            /* set value if there is such*/
            if (reader->has_string_value == true) {
                /*skip spaces and >*/
                while( *(line) == ' ' && *(line) == '>')
                    line++;

                reader->parsed_line.value = line;

                while( *(line) != '<')
                        line++;

                *(line) = '\0';
            }

            break;
        case Document: /*xml header*/
            while( *(line) == '<' ||  *(line) == '?' )
                line++;

            reader->parsed_line.name = line;

            while( *(line) != ' ' && (*(line) != '?' && *(line) != '>'))
               line++;

            *(line) = '\0';
            line++;

            /*set attributes*/
            if (reader->attr_count > 0) {
                for (attr_i = 0 ; attr_i < reader->attr_count; attr_i++) {

                    /*skip spaces*/
                    while( *(line) == ' ')
                        line++;

                    reader->parsed_line.attr_names[attr_i] = line;

                    while( *(line) != '=')
                        line++;

                    *(line) = '\0';
                    line++;
                    if (*(line) == '"')  line++; /* skip quotes*/

                    reader->parsed_line.attr_values[attr_i] = line;

                    while( *(line) != '"')
                        line++;

                    *(line) = '\0';
                }
            }

            /* set value if there is such*/
            if (reader->has_string_value == true) {
                /*skip spaces and >*/
                while( *(line) == ' ' && *(line) == '>')
                    line++;

                reader->parsed_line.value = line;

                while( *(line) != '<')
                        line++;

                *(line) = '\0';
            }

            break;
        case Comment: /*user comment */
            while( *(line) == '<' || *(line) == '-' || *(line) == ' ' )
                line++;

            reader->parsed_line.name = line;

            while( *(line) != '-' && *(line+1) != '-' && *(line+2) != '>' )
               line++;

            break;
        case EndElement: /*close tag*/
            while( *(line) == '<' ||  *(line) == '/')
                line++;

            reader->parsed_line.name = line;

            while(*(line) != '>')
               line++;

            break;
        case None:
            break;
    }
    reader->current_pos = (line + 1);

    *(line) = '\0';


    return true;
}

