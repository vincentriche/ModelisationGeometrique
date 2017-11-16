#include "o_objet.h"
#include "t_geometrie.h"
#include <GL/gl.h>

struct my_bezier
{
    Table_quadruplet ctrlPts; // Points de contrôle
    Table_triplet splPts; // Points d'affichage
    Table_triplet drPts; // Dérivées
    Flottant pas; // Pas
    int displayCtrlPts; // Paramètre d'afffichage des points de contrôle
    Flottant intervalBegin; // Intervale début
    Flottant intervalEnd; // Intervale fin
};
static void deCasteljau(struct my_bezier *b);

static void affiche_bezier(struct my_bezier *b)
{
    // Affichage de la courbe de bézier
    glBegin(GL_LINES);
    for (int i = 1; i < b->splPts.nb - 1; i++)
    {
        // Affichage segments
        glColor3f(1.0, 0.0, 1.0);
        glVertex3f(b->splPts.table[i].x, b->splPts.table[i].y, b->splPts.table[i].z);
        glVertex3f(b->splPts.table[i + 1].x, b->splPts.table[i + 1].y, b->splPts.table[i + 1].z);

        // Affichage dérivées
        glColor3f(1.0, 1.0, 0.0);
        glVertex3f(b->splPts.table[i].x, b->splPts.table[i].y, b->splPts.table[i].z);
        glVertex3f(b->splPts.table[i].x + b->drPts.table[i].x, 
                   b->splPts.table[i].y + b->drPts.table[i].y, 
                   b->splPts.table[i].z + b->drPts.table[i].z);
    }
    glEnd();

    // Affichage des points de contrôle (sous forme de ligne brisée)
    if(b->displayCtrlPts == 1)
    {
        glBegin(GL_LINES);
        glColor3f(255.0, 255.0, 0.0);
        for (int i = 0; i < b->ctrlPts.nb - 1; i++)
        {
            glVertex3f(b->ctrlPts.table[i].x, b->ctrlPts.table[i].y, b->ctrlPts.table[i].z);
            glVertex3f(b->ctrlPts.table[i + 1].x, b->ctrlPts.table[i + 1].y, b->ctrlPts.table[i + 1].z);
        }
        glEnd();
    }
}

static void changement(struct my_bezier *b)
{
    if(!(UN_CHAMP_CHANGE(b) || CREATION(b)))
        return;

    if(CHAMP_CHANGE(b, ctrlPts) 
        || CHAMP_CHANGE(b, pas) 
        || CHAMP_CHANGE(b, intervalBegin) 
        || CHAMP_CHANGE(b, intervalEnd) 
        || CHAMP_CHANGE(b, displayCtrlPts))
    {
        if(b->splPts.nb > 0)
            free(b->splPts.table);
        deCasteljau(b);
    }
}

static void deCasteljau(struct my_bezier *b)
{
    Table_quadruplet b_ctrlPts;
    ALLOUER(b_ctrlPts.table, b->ctrlPts.nb);

    float length = b->intervalEnd - b->intervalBegin; 
    ALLOUER(b->splPts.table, length / b->pas);
    ALLOUER(b->drPts.table, length / b->pas);
    b->splPts.nb = length / b->pas;
    b->drPts.nb = length / b->pas;
    int h = 0;

    Quadruplet tmpP0;
    Quadruplet tmpP1;
    for (float u = b->intervalBegin; h < b->splPts.nb; u += b->pas)
    {     
        // On met les points de contrôles en coordonées homogênes
        for (int i = 0; i < b->ctrlPts.nb; i++)
        {
            b_ctrlPts.table[i].x = b->ctrlPts.table[i].x * b->ctrlPts.table[i].h;
            b_ctrlPts.table[i].y = b->ctrlPts.table[i].y * b->ctrlPts.table[i].h;
            b_ctrlPts.table[i].z = b->ctrlPts.table[i].z * b->ctrlPts.table[i].h;
            b_ctrlPts.table[i].h = b->ctrlPts.table[i].h;
        }

        for (int i = 1; i < b->ctrlPts.nb; i++)
        {
            for (int j = 0; j < b->ctrlPts.nb - i; j++)
            {
                tmpP0 = b_ctrlPts.table[j];
                tmpP1 = b_ctrlPts.table[j + 1];

                b_ctrlPts.table[j].x = (1 - u) * b_ctrlPts.table[j].x + u * b_ctrlPts.table[j + 1].x;
                b_ctrlPts.table[j].y = (1 - u) * b_ctrlPts.table[j].y + u * b_ctrlPts.table[j + 1].y;
                b_ctrlPts.table[j].z = (1 - u) * b_ctrlPts.table[j].z + u * b_ctrlPts.table[j + 1].z;
                b_ctrlPts.table[j].h = (1 - u) * b_ctrlPts.table[j].h + u * b_ctrlPts.table[j + 1].h;
            }
        }
        b->splPts.table[h].x = b_ctrlPts.table[0].x / b_ctrlPts.table[0].h;
        b->splPts.table[h].y = b_ctrlPts.table[0].y / b_ctrlPts.table[0].h;
        b->splPts.table[h].z = b_ctrlPts.table[0].z / b_ctrlPts.table[0].h;

        tmpP0.x = tmpP0.x / b_ctrlPts.table[0].h;
        tmpP0.y = tmpP0.y / b_ctrlPts.table[0].h;
        tmpP0.z = tmpP0.z / b_ctrlPts.table[0].h;

        tmpP1.x = tmpP1.x / b_ctrlPts.table[0].h;
        tmpP1.y = tmpP1.y / b_ctrlPts.table[0].h;
        tmpP1.z = tmpP1.z / b_ctrlPts.table[0].h;

        b->drPts.table[h].x = tmpP1.x - tmpP0.x;
        b->drPts.table[h].y = tmpP1.y - tmpP0.y;
        b->drPts.table[h].z = tmpP1.z - tmpP0.z;
        h++;        
    }
    free(b_ctrlPts.table);
}

CLASSE(bezier, struct my_bezier,
        CHAMP(ctrlPts, L_table_point P_table_quadruplet Extrait Sauve)
        CHAMP(pas, L_flottant Edite DEFAUT("0.1"))
        CHAMP(displayCtrlPts, L_entier Edite DEFAUT("0"))
        CHAMP(intervalBegin, L_flottant Edite DEFAUT("0.0"))
        CHAMP(intervalEnd, L_flottant Edite DEFAUT("1.0"))
        CHANGEMENT(changement)
        CHAMP_VIRTUEL(L_affiche_gl(affiche_bezier))
        EVENEMENT("Shft+B+E")
        MENU("Modélisation Géométrique/Courbe de Bézier")
        )
