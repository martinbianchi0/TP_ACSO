#include "ej1.h"

/**
 * @brief Crea una lista vacía de procesamiento de strings.
 * 
 * @return Un puntero a la nueva lista o NULL si ocurrió un error al asignar memoria.
 */
string_proc_list* string_proc_list_create(void){
	string_proc_list* lista = (string_proc_list*)malloc(sizeof(string_proc_list));
	if (lista == NULL) {
		return NULL;  // error de asignación
	}
	lista->first = NULL;
	lista->last = NULL;
	return lista;
}

/**
 * @brief Crea un nodo con el tipo y hash especificados.
 * 
 * @param type Tipo del nodo (uint8_t).
 * @param hash Puntero al hash (se almacena como referencia, no se copia).
 * @return Un puntero al nuevo nodo o NULL si ocurrió un error al asignar memoria.
 */
string_proc_node* string_proc_node_create(uint8_t type, char* hash){
	string_proc_node* nodo = (string_proc_node*)malloc(sizeof(string_proc_node));
	if (nodo == NULL) {
		return NULL;  // error de asignación
	}

	nodo->next = NULL;
	nodo->previous = NULL;
	nodo->type = type;
	nodo->hash = hash;
	return nodo;
}

/**
 * @brief Agrega un nodo al final de la lista con el tipo y hash especificados.
 * 
 * @param list Puntero a la lista donde se agregará el nodo.
 * @param type Tipo del nodo (uint8_t).
 * @param hash Puntero al hash (se almacena como referencia, no se copia).
 */
void string_proc_list_add_node(string_proc_list* list, uint8_t type, char* hash){
	if (list == NULL) {
        return;
    }

	string_proc_node* nodo = string_proc_node_create(type, hash);
	if (nodo == NULL) {
		return; // error de asignación
	}

	if (list->first == NULL) {
		list->first = nodo;
		list->last = nodo;
	} else {
		nodo->previous = list->last;
		list->last->next = nodo;
		list->last = nodo;
	}
}

/**
 * @brief Genera un nuevo hash concatenando el hash dado con los hashes
 * de los nodos de la lista que coincidan con el tipo especificado.
 * 
 * @param list Puntero a la lista de nodos.
 * @param type Tipo de nodos a considerar para la concatenación.
 * @param hash Hash inicial que será el punto de partida de la concatenación.
 * @return Un puntero al nuevo hash concatenado o NULL si ocurrió un error.
 *         El resultado debe liberarse con free() después de su uso.
 */
char* string_proc_list_concat(string_proc_list* list, uint8_t type , char* hash){
    if (list == NULL || hash == NULL) {
        return NULL;
    }

    char* result = str_concat("", hash);
    if (result == NULL) return NULL;

    string_proc_node* current_node = list->first;
    while (current_node != NULL) {
        if (current_node->type == type) {
            char* temp = str_concat(result, current_node->hash);
            if (temp == NULL) {
                free(result);
                return NULL;
            }
            free(result);
            result = temp;
        }
        current_node = current_node->next;
    }
    return result;
}


/** AUX FUNCTIONS **/

void string_proc_list_destroy(string_proc_list* list){

	/* borro los nodos: */
	string_proc_node* current_node	= list->first;
	string_proc_node* next_node		= NULL;
	while(current_node != NULL){
		next_node = current_node->next;
		string_proc_node_destroy(current_node);
		current_node	= next_node;
	}
	/*borro la lista:*/
	list->first = NULL;
	list->last  = NULL;
	free(list);
}
void string_proc_node_destroy(string_proc_node* node){
	node->next      = NULL;
	node->previous	= NULL;
	node->hash		= NULL;
	node->type      = 0;			
	free(node);
}


char* str_concat(char* a, char* b) {
	int len1 = strlen(a);
    int len2 = strlen(b);
	int totalLength = len1 + len2;
    char *result = (char *)malloc(totalLength + 1); 
    strcpy(result, a);
    strcat(result, b);
    return result;  
}

void string_proc_list_print(string_proc_list* list, FILE* file){
        uint32_t length = 0;
        string_proc_node* current_node  = list->first;
        while(current_node != NULL){
                length++;
                current_node = current_node->next;
        }
        fprintf( file, "List length: %d\n", length );
		current_node    = list->first;
        while(current_node != NULL){
                fprintf(file, "\tnode hash: %s | type: %d\n", current_node->hash, current_node->type);
                current_node = current_node->next;
        }
}

