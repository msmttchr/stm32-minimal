#ifndef __ROUTING_H__
#define __ROUTING_H__
void routing_init(void);
int routing_connection(char ep1, char ep2);
int routing_connection_query(char *endpoints, char *reply_buffer, int active);
int routing_disconnection(char ep1, char ep2);
void routing_disconnect_all(void);
int routing_idle(void);
#endif /* __ROUTING_H__ */
