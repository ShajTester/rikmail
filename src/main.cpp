


#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cmath>
// Отсюда
// https://www.tutorialspoint.com/cplusplus/cpp_signal_handling.htm
#include <csignal>
#include <syslog.h>

#include <vector>
#include <functional>

#include <filesystem>

// #include <unistd.h>
// #include <sys/socket.h>
// #include <sys/un.h>
// #include <sys/stat.h>
// #include <fcntl.h>

#include <nlohmann/json.hpp>

/// gDBus
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "rikmail-manager.h"

#include "CSmtp.h"


namespace fs = std::filesystem;
using namespace std::literals::chrono_literals;
using json = nlohmann::json;

// #define RIKFAN_DEBUG


class NotImplementedException : public std::logic_error
{
public:
    NotImplementedException() : std::logic_error("Function not implemented yet") { };
};


void rikmail_set_timer(const std::string &time_str)
{
    constexpr auto send_report_timer = "/lib/systemd/system/send-report.timer";
    // constexpr auto dest = "org.freedesktop.systemd1";
    // constexpr auto path = "/org/freedesktop/systemd1";
    // constexpr auto interface = "org.freedesktop.systemd1.Manager";

    std::string cmd = "sed -i 's/OnCalendar.*/OnCalendar=";
    std::for_each(time_str.begin(), time_str.end(), [&cmd](const char c)
            {
                if(c == '*')
                    cmd += "\\*";
                else
                    cmd += c;
            });
    cmd += "/g' ";
    cmd += send_report_timer;
    syslog(LOG_INFO, "%s", cmd.c_str());

    system(cmd.c_str());
    system("systemctl daemon-reload");
    system("systemctl restart send-report.timer");
    // throw NotImplementedException();
}



static gboolean on_handle_apply_SMTPParams (XyzOpenbmc_projectAresRikmail *interface,
        GDBusMethodInvocation *invocation,
        const gchar *addr,
        gboolean ev1,
        gboolean ev2,
        gboolean ev3,
        gboolean ev4,
        gboolean ev5,
        gboolean ev6,
        gboolean ev7,
        gboolean ev8
                                           )
{
    std::string out_email;
    std::stringstream report_str;

    // syslog(LOG_INFO, "%s", __PRETTY_FUNCTION__);
    syslog(LOG_INFO, "greeting:5:: %s", addr);

    if (strcmp(addr, "start") == 0)
    {
        out_email = "test@example.com";
        ev1 = FALSE;
        ev2 = FALSE;
        ev3 = FALSE;
        ev4 = FALSE;
        ev5 = FALSE;
        ev6 = FALSE;
        ev7 = FALSE;
        ev8 = FALSE;
        report_str << "Initial call. Retreive data from config." << std::endl;
    }
    else
    {
        // Проверка введенного адреса
        out_email = addr;
        // Обработка логики
        if (ev1)
        {
            // Послать тестовое сообщение
            syslog(LOG_INFO, "Sending message to %s", addr);


            {
                bool bError = false;

                try
                {
                    CSmtp mail;

                    mail.SetSMTPServer("localhost", 25);
                    // mail.SetLogin("***");
                    // mail.SetPassword("***");
                    mail.SetSenderName("root");
                    mail.SetSenderMail("root.ares@rikor.com");
                    mail.SetReplyTo("root.ares@rikor.com");
                    mail.SetSubject("Test message");
                    mail.AddRecipient(out_email.c_str());
                    // mail.SetXPriority(XPRIORITY_NORMAL);
                    // mail.SetXMailer("The Bat! (v3.02) Professional");
                    mail.AddMsgLine("Hello,");
                    mail.AddMsgLine("");
                    mail.AddMsgLine("How are you today?");
                    mail.AddMsgLine("");
                    mail.AddMsgLine("Regards");
                    mail.AddMsgLine("--");
                    mail.AddMsgLine("User");
                    // mail.AddAttachment("c:\\test.exe");
                    // mail.AddAttachment("c:\\test2.jpg");

                    mail.Send();
                    report_str << "Send test message to " << out_email << std::endl;
                }
                catch (ECSmtp e)
                {
                    // std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
                    syslog(LOG_ERR, "Error: %s", e.GetErrorText().c_str());
                    bError = true;
                }

                if (!bError)
                {
                    // std::cout << "Mail was send successfully.\n";
                    syslog(LOG_INFO, "Mail was send successfully.");
                }
            }
            

            ev1 = FALSE;
        }
        if(ev2)
        {
            // Настройка таймера: *-*-* *:00,30:00
            try
            {
                rikmail_set_timer("*-*-* *:00,30:00");
                report_str << "Set timer to 2 times of hour" << std::endl;
            }
            catch (const std::exception &e)
            {
                report_str << "Error on set timer: " << e.what() << std::endl;
            }
            ev3 = FALSE;
            ev4 = FALSE;
        }
        else if(ev3)
        {
            // Настройка таймера: *-*-* 22:15:00
            try
            {
                rikmail_set_timer("*-*-* 22:15:00");
                report_str << "Set timer to 1 times of day" << std::endl;
            }
            catch (const std::exception &e)
            {
                report_str << "Error on set timer: " << e.what() << std::endl;
            }
            ev2 = FALSE;
            ev4 = FALSE;
        }
        else if(ev4)
        {
            // Настройка таймера: *-*-* *:00/15:00
            try
            {
                rikmail_set_timer("*-*-* *:00/15:00");
                report_str << "Set timer to 4 times of hour" << std::endl;
            }
            catch (const std::exception &e)
            {
                report_str << "Error on set timer: " << e.what() << std::endl;
            }
            ev2 = FALSE;
            ev3 = FALSE;
        }

    }

    // https://developer.gnome.org/glib/stable/glib-GVariant.html
    // https://people.gnome.org/~ryanl/glib-docs/gvariant-format-strings.html
    // Правильная работа с GVariant
    // https://stackoverflow.com/a/46507056
    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("av"));
    g_variant_builder_add (&builder, "v", g_variant_new_string (out_email.c_str()));
    g_variant_builder_add (&builder, "v", g_variant_new_boolean (ev1));
    g_variant_builder_add (&builder, "v", g_variant_new_boolean (ev2));
    g_variant_builder_add (&builder, "v", g_variant_new_boolean (ev3));
    g_variant_builder_add (&builder, "v", g_variant_new_boolean (ev4));
    g_variant_builder_add (&builder, "v", g_variant_new_boolean (ev5));
    g_variant_builder_add (&builder, "v", g_variant_new_boolean (ev6));
    g_variant_builder_add (&builder, "v", g_variant_new_boolean (ev7));
    g_variant_builder_add (&builder, "v", g_variant_new_boolean (ev8));
    g_variant_builder_add (&builder, "v", g_variant_new_string (report_str.str().c_str()));
    g_variant_builder_close (&builder);



    g_autoptr (GVariant) output = g_variant_builder_end (&builder);

    xyz_openbmc_project_ares_rikmail_complete_smtpparams (interface, invocation, output);
    // xyz_openbmc_project_ares_rikmail_complete_smtpparams (interface, invocation, "test@test.com");



    return TRUE;
}



static void on_bus_acquired (GDBusConnection *connection,
                             const gchar     *name,
                             gpointer         user_data)
{
    XyzOpenbmc_projectAresRikmail *interface;
    GError *error;

    /* This is where we'd export some objects on the bus */
    syslog(LOG_INFO, "on_bus_acquired %s on the session bus\n", name);


    gchar *conn_name;
    g_object_get(connection, "unique-name", &conn_name, NULL);
    syslog(LOG_INFO, "%s\n", conn_name);
    g_free(conn_name);

    interface = xyz_openbmc_project_ares_rikmail_skeleton_new();
    g_signal_connect (interface, "handle-smtpparams",
                      G_CALLBACK (on_handle_apply_SMTPParams),
                      NULL);
    error = NULL;
    if (!g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (interface), connection, "/xyz/openbmc_project/ares/rikmail", &error))
    {
        g_print("ERROR %s\n", error->message);
    }
}

static void on_name_lost (GDBusConnection *connection,
                          const gchar     *name,
                          gpointer         user_data)
{
    syslog(LOG_ERR, "on_name_lost %s on the session bus\n", name);
}


static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    // syslog(LOG_INFO, "on_name_acquired %s on the session bus\n", name);
}




static  GMainLoop *loop;

/*
 * On SIGINT, exit the main loop
 */
static void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        g_main_loop_quit(loop);
    }
}



int main(int argc, char const *argv[])
{
    openlog("rikmail", LOG_CONS, LOG_USER);

    // set up the SIGINT signal handler
    if (signal(SIGINT, &sig_handler) == SIG_ERR)
    {
        syslog(LOG_INFO, "Failed to register SIGINT handler, quitting...\n");
        exit(EXIT_FAILURE);
    }

    loop = g_main_loop_new (NULL, FALSE);
    guint bus_id = g_bus_own_name(G_BUS_TYPE_SYSTEM,
                                  "xyz.openbmc_project.ares.rikmail",
                                  G_BUS_NAME_OWNER_FLAGS_NONE,
                                  on_bus_acquired,
                                  on_name_acquired,
                                  on_name_lost,
                                  NULL, NULL);

    // syslog(LOG_INFO, "Initial PID: %d\n", getpid());
    // syslog(LOG_INFO, "bus_id %u\n", bus_id);

    g_main_loop_run (loop);
    g_bus_unown_name(bus_id);

    g_main_loop_unref(loop);

    syslog(LOG_INFO, "Stop rikmail");

    return 0;
}
