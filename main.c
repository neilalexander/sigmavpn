//
//  main.c
//  Sigma main code
//
//  Copyright (c) 2011, Neil Alexander T.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with
//  or without modification, are permitted provided that the following
//  conditions are met:
//
//  - Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

#include "types.h"
#include "modules.h"
#include "dep/ini.h"

#define THREAD_STACK_SIZE 131072

sigma_session* sessions = NULL;

static int handler(void* user, const char* section, const char* name, const char* value)
{
    if (section[0] == '!')
        return 0;

    (void) user;

    if (name == NULL && value == NULL)
    {
        sigma_session** newobject = &sessions;

        while (*newobject)
        {
            if (strncmp((*newobject)->sessionname, section, 32) == 0)
                return 0;

            newobject = &((*newobject)->next);
        }

        *newobject = calloc(1, sizeof(sigma_session));
        strncpy((*newobject)->sessionname, section, 32);
    }
        else
    {
        sigma_session* pointer = sessions;

        while (pointer)
        {
            if (strncmp(pointer->sessionname, section, 32) == 0)
                break;

            pointer = pointer->next;
        }

        if (pointer == NULL)
        {
            fprintf(stderr, "Session %s not found (this should never happen)\n", section);
            return -1;
        }

        if (strcmp(name, "proto") == 0)
        {
            if (!pointer->proto)
                pointer->proto = loadproto((char*) value);
        }
            else
        if (strncmp(name, "proto_", 6) == 0)
        {
            if (pointer->proto == NULL)
            {
                fprintf(stderr, "%s: Parameter '%s' ignored; 'proto=' should appear before '%s=' in the config\n", pointer->sessionname, name, name);
                return -1;
            }

            pointer->proto->set(pointer->proto, name + 6, value);
        }

        if (strcmp(name, "peer") == 0)
        {
            if (!pointer->remote)
                pointer->remote = loadinterface((char*) value);
        }
            else
        if (strncmp(name, "peer_", 5) == 0)
        {
            if (pointer->remote == NULL)
            {
                fprintf(stderr, "%s: Parameter '%s' ignored; 'peer=' should appear before '%s=' in the config\n", pointer->sessionname, name, name);
                return -1;
            }

            pointer->remote->set(pointer->remote, name + 5, value);
        }

        if (strcmp(name, "local") == 0)
        {
            if (!pointer->local)
                pointer->local = loadinterface((char*) value);
        }
            else
        if (strncmp(name, "local_", 6) == 0)
        {
            if (pointer->local == NULL)
            {
                fprintf(stderr, "%s: Parameter '%s' ignored; 'local=' should appear before '%s=' in the config\n", pointer->sessionname, name, name);
                return -1;
            }

            pointer->local->set(pointer->local, name + 6, value);
        }
    }

    return 0;
}

void reload()
{
    printf("Reloading configuration...\n");

    if (ini_parse(conf->configfile, handler, (void*) NULL) < 0)
    {
        printf("Configuration file '%s' could not be parsed.\n", conf->configfile);
        return;
    }

    sigma_session* pointer = sessions;

    char buffer = 'R';

    while (pointer)
    {
        if (write(pointer->controlpipe[1], &buffer, 1) < 0)
		perror("write");

        pointer = pointer->next;
    }

    printf("Configuration reloaded.\n");
}

int main(int argc, const char** argv)
{
    printf("SigmaVPN.\nCopyright (c) 2011 Neil Alexander T. All rights reserved.\n");

    conf = malloc(sizeof(sigma_conf));
    strncpy(conf->modulepath, "/usr/local/lib/sigmavpn/", 128);
    strncpy(conf->configfile, "/usr/local/etc/sigmavpn.conf", 128);

    int arg;

    for (arg = 1; arg < argc; arg ++)
    {
        if (argv[arg][0] != '-')
            continue;

        switch ((int) argv[arg][1])
        {
            case '?':
            {
                printf("Possible arguments:\n\t-c 'path/to/config.conf'\n\t-m 'path/to/module/folder'\n");
                return -1;
            }

            case 'c':
            {
                if (argv[arg + 1] == NULL)
                {
                    fprintf(stderr, "Expected configuration path after '-c'\n");
                    continue;
                }

                strncpy(conf->configfile, argv[arg + 1], 128);
                printf("Using configuration file '%s'\n", argv[arg + 1]);
                arg ++;

                break;
            }

            case 'm':
            {
                if (argv[arg + 1] == NULL)
                {
                    fprintf(stderr, "Expected module path after '-m'\n");
                    continue;
                }

                strncpy(conf->modulepath, argv[arg + 1], 128);
                printf("Using module path '%s'\n", argv[arg + 1]);
                arg ++;

                break;
            }

            default:
                fprintf(stderr, "Unknown argument '%s'\n", argv[arg]);
        }
    }

    if (ini_parse(conf->configfile, handler, (void*) NULL) < 0)
    {
        printf("Configuration file '%s' could not be parsed\n", conf->configfile);
        return 1;
    }

    //pthread_attr_t attr;
    //pthread_attr_init(&attr);
    //pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);

    sigma_session* pointer = sessions;

    if (pointer == NULL)
    {
        fprintf(stderr, "No session data found; please check the configuration.\n");
        return -1;
    }

    while (pointer)
    {
        int rc = pipe(pointer->controlpipe);
        if (rc)
        {
            fprintf(stderr, "pipe() returned %d\n", rc);
            return -1;
        }

        rc = pthread_create(&(pointer->thread), 0, sessionwrapper, pointer);

        if (rc)
        {
            fprintf(stderr, "Thread returned %d\n", rc);
            return -1;
        }

        pointer = pointer->next;
    }

    struct sigaction act;
    memset(&act, '\0', sizeof(struct sigaction));
    act.sa_sigaction = &reload;
    act.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(SIGHUP, &act, NULL) < 0)
        perror("sigaction");

    pointer = sessions;

    while (pointer)
    {
        pthread_join(pointer->thread, NULL);
        pointer = pointer->next;
    }

    return 0;
}

void* sessionwrapper(void* param)
{
    int status = runsession((sigma_session*) param);
    pthread_exit(&status);
}

int max(int a, int b)
{
    return a > b ? a : b;
}

int reloadsession(sigma_session* session, char operation)
{
    switch (operation)
    {
        default:
            if (session->proto->reload != NULL)
            {
                printf("Restarting protocol...");
                if (session->proto->reload(session->proto) == 0) printf(" done.\n"); else printf(" failed.\n");
            }

            if (session->local->reload != NULL)
            {
                printf("Restarting local interface...");
                if (session->local->reload(session->local) == 0) printf(" done.\n"); else printf(" failed.\n");
            }

            if (session->remote->reload != NULL)
            {
                printf("Restarting remote interface...");
                if (session->remote->reload(session->remote) == 0) printf(" done.\n"); else printf(" failed.\n");
            }

            break;
    }

    return 0;
}

int runsession(sigma_session* session)
{
    fd_set sockets;

    if (session->proto == NULL)
    {
        fprintf(stderr, "%s: Protocol not loaded, configuration error?\n", session->sessionname);
        return -1;
    }

    if (session->local == NULL)
    {
        fprintf(stderr, "%s: Local interface not loaded, configuration error?\n", session->sessionname);
        return -1;
    }

    if (session->remote == NULL)
    {
        fprintf(stderr, "%s: Peer interface not loaded, configuration error?\n", session->sessionname);
        return -1;
    }

    if (session->proto->init(session->proto) == -1)
    {
        fprintf(stderr, "%s: Protocol initalisation failed, session not loaded\n", session->sessionname);
        return -1;
    }

    if (session->local->init(session->local) == -1)
    {
        fprintf(stderr, "%s: Local interface initialisation failed, session not loaded\n", session->sessionname);
        return -1;
    }

    if (session->remote->init(session->remote) == -1)
    {
        fprintf(stderr, "%s: Peer interface initialisation failed, session not loaded\n", session->sessionname);
        return -1;
    }

    printf("%s: Session active\n", session->sessionname);

    while (1)
    {
        FD_ZERO(&sockets);
        FD_SET(session->local->filedesc, &sockets);
        FD_SET(session->remote->filedesc, &sockets);
        FD_SET(session->controlpipe[0], &sockets);

        int nfds = max(session->local->filedesc, session->remote->filedesc);
        nfds = max(nfds, session->controlpipe[0]);
        nfds++;

        int len = select(nfds, &sockets, NULL, NULL, 0);

        if (len < 0)
        {
            fprintf(stderr, "Poll error");
            return -1;
        }

        if (FD_ISSET(session->controlpipe[0], &sockets) != 0)
        {
            char buffer;
            long readvalue = read(session->controlpipe[0], &buffer, 1);

            if (readvalue < 0)
            {
                fprintf(stderr, "%s: Control read error %ld: %s\n", session->sessionname, readvalue, strerror(errno));
                return -1;
            }

            readvalue = reloadsession(session, buffer);
            if (readvalue < 0) return readvalue;
            continue;
        }

        if (FD_ISSET(session->local->filedesc, &sockets) != 0)
        {
            char tuntapbuf[MAX_BUFFER_SIZE], tuntapbufenc[MAX_BUFFER_SIZE];
            long readvalue = session->local->read(session->local, tuntapbuf, MAX_BUFFER_SIZE);

            if (readvalue < 0)
            {
                fprintf(stderr, "%s: Local read error %ld: %s\n", session->sessionname, readvalue, strerror(errno));
                return -1;
            }

            readvalue = session->proto->encode(session->proto, tuntapbuf, tuntapbufenc, readvalue);

            if (readvalue < 0)
            {
                fprintf(stderr, "%s: Local read error %ld: %s\n", session->sessionname, readvalue, strerror(errno));
                return -1;
            }

            long writevalue = session->remote->write(session->remote, tuntapbufenc, readvalue);

            if (writevalue < 0)
            {
                fprintf(stderr, "%s: Local write error %ld: %s\n", session->sessionname, writevalue, strerror(errno));
                return -1;
            }
        }

        if (FD_ISSET(session->remote->filedesc, &sockets) != 0)
        {
            char udpbuf[MAX_BUFFER_SIZE], udpbufenc[MAX_BUFFER_SIZE];
            long readvalue = session->remote->read(session->remote, udpbufenc, MAX_BUFFER_SIZE);

            if (readvalue < 0)
            {
                fprintf(stderr, "%s: Remote read error %ld: %s\n", session->sessionname, readvalue, strerror(errno));
                return -1;
            }

            readvalue = session->proto->decode(session->proto, udpbufenc, udpbuf, readvalue);

            if (readvalue < 0)
            {
                fprintf(stderr, "%s: Remote read error %ld: %s\n", session->sessionname, readvalue, strerror(errno));
                return -1;
            }

            long writevalue = session->local->write(session->local, udpbuf, readvalue);

            if (writevalue < 0)
            {
                fprintf(stderr, "%s: Remote write error %ld: %s\n", session->sessionname, writevalue, strerror(errno));
                return -1;
            }

            if (session->remote->updateremote != NULL)
            {
                session->remote->updateremote(session->remote);
            }
        }
    }

    return 0;
}
