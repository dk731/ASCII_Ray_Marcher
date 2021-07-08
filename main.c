#include "helper.h"

float hor_fow = 45.0f * D2RAD;

double camera_sens = 1 * D2RAD;

double move_speed = 0.1;
double sprint_mult = 1.5;

long last_screen = -1;

buffer *sbuf;
camera main_camera;

void screenshot_callb(char c)
{

    if (millis() - last_screen >= 3000) // allow screenshot only once in 3 sec
    {
        high_res_screenshot(&main_camera);
        last_screen = millis();
    }
}

int main()
{
    init_lib();
    init_input();
    callback ss_cb = {.func = screenshot_callb, .pattern = "pP"};
    add_callback(&ss_cb);
    int w = 90, h = 30;
    sbuf = init_draw_buf(w, h);

    SET_VEC3(main_camera.pos, -5.0, 0.0, 0.0);
    SET_VEC3(main_camera.direction_ang, 0.0, 0.0, 0.0);
    SET_VEC2(main_camera.fov, hor_fow, (hor_fow * h) / w * 1.5);

    bool running = true;
    while (running)
    {
        pthread_mutex_lock(&inp_lock);
        vec3 move_len = VEC3_ZERO;
        while (inp.top >= 0)
        {
            switch (inp.inp_chars[inp.top--])
            {
            case 1: // [1, 4] - camera rotation: up, down, right, left
                main_camera.direction_ang.y -= camera_sens;
                break;
            case 2:
                main_camera.direction_ang.y += camera_sens;
                break;
            case 3:
                main_camera.direction_ang.x += camera_sens;
                break;
            case 4:
                main_camera.direction_ang.x -= camera_sens;
                break;
            case 'w':
                move_len.x += move_speed;
                break;
            case 's':
                move_len.x -= move_speed;
                break;
            case 'd':
                move_len.y += move_speed;
                break;
            case 'a':
                move_len.y -= move_speed;
                break;
            case 'W':
                move_len.x += move_speed * sprint_mult;
                break;
            case 'S':
                move_len.x -= move_speed * sprint_mult;
                break;
            case 'D':
                move_len.y += move_speed * sprint_mult;
                break;
            case 'A':
                move_len.y -= move_speed * sprint_mult;
                break;
            case 32: // space
                move_len.z += move_speed;
                break;
            case 'z':
            case 'Z':
                move_len.z -= move_speed;
                break;
            case 'e':
            case 'E':
                main_camera.direction_ang.z += camera_sens;
                break;
            case 'q':
            case 'Q':
                main_camera.direction_ang.z -= camera_sens;
                break;
            case 27: // escape
                running = false;
                break;
            }
        }
        pthread_mutex_unlock(&inp_lock);

        vec3 dir = main_camera.direction_ang;
        main_camera.trans_mat = (double[]){
            cos(dir.x) * cos(dir.y),
            -sin(dir.x),
            cos(dir.x) * sin(dir.y),
            sin(dir.x) * cos(dir.y),
            cos(dir.x),
            sin(dir.x) * sin(dir.y),
            -sin(dir.y),
            0.0,
            cos(dir.y)};

        vec3 for_vec = VEC3_FORWARD;
        vec3 pos_offset;
        cblas_dgemv(CblasRowMajor, CblasNoTrans, 3, 3, 1.0, main_camera.trans_mat, 3, &(for_vec.x), 1, 0.0, &(main_camera.direction_vec.x), 1);
        cblas_dgemv(CblasRowMajor, CblasNoTrans, 3, 3, 1.0, main_camera.trans_mat, 3, &(move_len.x), 1, 0.0, &(pos_offset.x), 1);
        cblas_daxpy(3, 1.0, &(pos_offset.x), 1, &(main_camera.pos.x), 1);

        render(sbuf, &main_camera);
        draw(sbuf, &main_camera);
    }

    clear_lib();
}