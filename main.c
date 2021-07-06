#include "helper.h"

char screen_buf[BUF_SIZE];

float hor_fow = 45.0f * D2RAD;

double camera_sens = 5 * D2RAD;

double move_speed = 0.1;
double sprint_mult = 1.5;

int main()
{
    init_lib(screen_buf);
    init_input(NULL);

    camera main_camera = {.direction_ang = {.x = 0.0f, .y = 0.0f, .z = 0.0f},
                          .fov = {.x = hor_fow, .y = SHEIGHT / (double)SWIDTH * hor_fow},
                          .pos = {.x = 0.0f, .y = 0.0f, .z = 0.0f}};
    bool running = true;
    while (running)
    {

        pthread_mutex_lock(&inp_lock);
        vec3 move_len = VEC3_ZERO;
        while (inp.top_ind > 0)
        {
            switch (inp.inp_chars[inp.top_ind--])
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
                move_len.z += move_speed;
                break;
            case 'a':
                move_len.z -= move_speed;
                break;
            case 'W':
                move_len.x += move_speed * sprint_mult;
                break;
            case 'S':
                move_len.x -= move_speed * sprint_mult;
                break;
            case 'D':
                move_len.z += move_speed * sprint_mult;
                break;
            case 'A':
                move_len.z -= move_speed * sprint_mult;
                break;
            case 32: // space
                move_len.y += move_speed;
                break;
            case 'z':
            case 'Z':
                move_len.y -= move_speed;
                break;
            case 'r':
            case 'R':
                main_camera.direction_ang.z += camera_sens;
                break;
            case 'q':
            case 'Q':
                main_camera.direction_ang.z -= camera_sens;
                break;
            case 27: // escape
                running = false;
            }
        }

        vec3 dir = main_camera.direction_ang;
        // cam->trans_mat = {};
        main_camera.trans_mat = (double[]){
            cos(dir.x) * cos(dir.y),
            cos(dir.x) * sin(dir.y) * sin(dir.z) - sin(dir.x) * cos(dir.z),
            cos(dir.x) * sin(dir.y) * cos(dir.z) + sin(dir.x) * sin(dir.z),
            sin(dir.x) * cos(dir.y),
            sin(dir.x) * sin(dir.y) * sin(dir.z) + cos(dir.x) * cos(dir.z),
            sin(dir.x) * sin(dir.y) * cos(dir.z) - cos(dir.x) * sin(dir.z),
            -sin(dir.y),
            cos(dir.y) * sin(dir.z),
            cos(dir.y) * cos(dir.z)};

        vec3 for_vec = VEC3_FORWARD;
        vec3 pos_offset;
        cblas_dgemv(CblasRowMajor, CblasNoTrans, 3, 3, 1.0, main_camera.trans_mat, 3, &(for_vec.x), 1, 0.0, &(main_camera.direction_vec.x), 1);
        cblas_dgemv(CblasRowMajor, CblasNoTrans, 3, 3, 1.0, main_camera.trans_mat, 3, &(move_len.x), 1, 0.0, &(pos_offset.x), 1);
        cblas_daxpy(3, 1.0, &(pos_offset.x), 1, &(main_camera.pos.x), 1);

        pthread_mutex_unlock(&inp_lock);

        render(screen_buf, &main_camera);
        draw(screen_buf);
    }

    clear_lib();
}