// PARIS AXEL
// RICHE VINCENT

#include "o_objet.h"
#include "t_geometrie.h"
#include <GL/gl.h>

struct my_bsurface
{
    Grille_quadruplet ctrlPts; // Points de contrôle
    Grille_quadruplet splPts; // Points d'affichage
    Flottant pas; // Pas
    int displayCtrlPts; // Paramètre d'affichage des points de contrôle
    Quadruplet interval; // Intervale début en U et V
};
static Quadruplet deCasteljau(Quadruplet*, int, float);
static void surface(struct my_bsurface*);

static void affiche_bsurface(struct my_bsurface *b)
{
    // Affichage de la courbe de bézier
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 1.0);
    for (int i = 0; i < b->splPts.nb_lignes; i++)
    {
        for (int j = 0; j < b->splPts.nb_colonnes; j++)
        {
            Quadruplet p1 = b->splPts.grille[i][j];

            if(i < b->splPts.nb_lignes - 1)
            {
                Quadruplet p2 = b->splPts.grille[i + 1][j];
                glVertex3f(p1.x, p1.y, p1.z);
                glVertex3f(p2.x, p2.y, p2.z);
            }

            if(j < b->splPts.nb_colonnes - 1)
            {
                Quadruplet p3 = b->splPts.grille[i][j + 1];
                glVertex3f(p1.x, p1.y, p1.z);
                glVertex3f(p3.x, p3.y, p3.z);
            }

        }   
    }        
    glEnd();

    // Affichage des points de contrôle (sous forme de ligne brisée)
    if(b->displayCtrlPts == 1)
    {
        glBegin(GL_LINES);
        glColor3f(1.0, 1.0, 0.0);
        for (int i = 0; i < b->ctrlPts.nb_lignes; i++)
        {
            for (int j = 0; j < b->ctrlPts.nb_colonnes; j++)
            {
                Quadruplet p1 = b->ctrlPts.grille[i][j];

                if(i < b->ctrlPts.nb_lignes - 1)
                {
                    Quadruplet p2 = b->ctrlPts.grille[i + 1][j];
                    glVertex3f(p1.x, p1.y, p1.z);
                    glVertex3f(p2.x, p2.y, p2.z);
                }

                if(j < b->ctrlPts.nb_colonnes - 1)
                {
                    Quadruplet p3 = b->ctrlPts.grille[i][j + 1];
                    glVertex3f(p1.x, p1.y, p1.z);
                    glVertex3f(p3.x, p3.y, p3.z);
                }

            }   
        }        
        glEnd();
    }
}

static void changement(struct my_bsurface *b)
{
    if(!(UN_CHAMP_CHANGE(b) || CREATION(b)))
        return;

    if(CHAMP_CHANGE(b, ctrlPts) 
        || CHAMP_CHANGE(b, pas) 
        || CHAMP_CHANGE(b, interval) 
        || CHAMP_CHANGE(b, displayCtrlPts))
    {
        if(b->splPts.nb_lignes > 0)
        {
            for (int i = 0; i < b->splPts.nb_lignes; i++)
            {
                free(b->splPts.grille[i]);
            }
            free(b->splPts.grille);
        }
        surface(b);
    }
}

static void surface(struct my_bsurface *b)
{  
    float lengthU = b->interval.y - b->interval.x;  
    int sizeU = (lengthU / b->pas) + 1; 

    float lengthV = b->interval.h - b->interval.z;  
    int sizeV = (lengthV / b->pas) + 1; 

    ALLOUER(b->splPts.grille, sizeU);
    for (int i = 0; i < sizeU; i++)
        ALLOUER(b->splPts.grille[i], sizeV);  
    b->splPts.nb_lignes = sizeU;
    b->splPts.nb_colonnes = sizeV;

    int nbLignes = b->ctrlPts.nb_lignes;
    int nbColonnes = b->ctrlPts.nb_colonnes;

    int h = 0;
    float u = b->interval.x;
    for (; h < sizeU; u += b->pas, h++)
    {
        int l = 0;
        float v = b->interval.z;
        for (; l < sizeV; v += b->pas, l++)
        {
            Quadruplet* tmpPts;
            ALLOUER(tmpPts, nbLignes);

            for (int a = 0; a < nbLignes; a++)
                tmpPts[a] = deCasteljau(b->ctrlPts.grille[a], nbColonnes, v);
            b->splPts.grille[h][l] = deCasteljau(tmpPts, nbLignes, u);
        }
    }
}

static Quadruplet deCasteljau(Quadruplet* linePts, int size, float u)
{      
    Quadruplet* ctrlLine;
    ALLOUER(ctrlLine, size);    
    
    // On met les points de contrôles en coordonées homogênes
    for (int i = 0; i < size; i++)
    {
        ctrlLine[i].x = linePts[i].x * linePts[i].h;
        ctrlLine[i].y = linePts[i].y * linePts[i].h;
        ctrlLine[i].z = linePts[i].z * linePts[i].h;
        ctrlLine[i].h = linePts[i].h;   
    }

    for (int i = 1; i < size; i++)
    {
        for (int j = 0; j < size - i; j++)
        {
            ctrlLine[j].x = (1 - u) * ctrlLine[j].x + u * ctrlLine[j + 1].x;
            ctrlLine[j].y = (1 - u) * ctrlLine[j].y + u * ctrlLine[j + 1].y;
            ctrlLine[j].z = (1 - u) * ctrlLine[j].z + u * ctrlLine[j + 1].z;
            ctrlLine[j].h = (1 - u) * ctrlLine[j].h + u * ctrlLine[j + 1].h;
        }
    }
    
    Quadruplet ret;
    ret.x = ctrlLine[0].x / ctrlLine[0].h;
    ret.y = ctrlLine[0].y / ctrlLine[0].h;
    ret.z = ctrlLine[0].z / ctrlLine[0].h;   
    ret.h = ctrlLine[0].h;   
    free(ctrlLine); 

    return ret; 
}

CLASSE(bsurface, struct my_bsurface,
        CHAMP(ctrlPts, L_grille_quadruplet P_grille_quadruplet Extrait Sauve)
        CHAMP(pas, L_flottant Edite DEFAUT("0.1"))
        CHAMP(displayCtrlPts, L_entier Edite DEFAUT("1"))
        CHAMP(interval, L_quadruplet Edite DEFAUT("0.0 1.0 0.0 1.0"))
        CHANGEMENT(changement)
        CHAMP_VIRTUEL(L_affiche_gl(affiche_bsurface))
        EVENEMENT("Shft+B+E")
        MENU("Modélisation Géométrique/Surface de Bézier")
        )
