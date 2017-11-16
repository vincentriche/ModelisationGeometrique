#include "o_objet.h"
#include "t_geometrie.h"
#include <GL/gl.h>

struct my_nurbs
{
    int degree;
    int displayCtrlPts;
    Flottant pas;
    Table_flottant seqNod;
    Table_quadruplet ctrlPts;
    Table_triplet splPts;
};
static void nurbs(struct my_nurbs *bs);
static void calculateSeqNode(struct my_nurbs *bs, int k);
static int calculateR(struct my_nurbs *bs, float u);

static void affiche_nurbs(struct my_nurbs *bs)
{
    // Affichage de la nurbs
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < bs->splPts.nb; i++)
    {
        glVertex3f(bs->splPts.table[i].x, 
                    bs->splPts.table[i].y, 
                    bs->splPts.table[i].z);
    }
    glEnd();

    // Affichage des points de contrôle (sous forme de ligne brisée)
    if(bs->displayCtrlPts == 1)
    {
        glBegin(GL_LINES);
        glColor3f(255.0, 255.0, 0.0);
        for (int i = 0; i < bs->ctrlPts.nb - 1; i++)
        {
            glVertex3f(bs->ctrlPts.table[i].x, bs->ctrlPts.table[i].y, bs->ctrlPts.table[i].z);
            glVertex3f(bs->ctrlPts.table[i + 1].x, bs->ctrlPts.table[i + 1].y, bs->ctrlPts.table[i + 1].z);
        }
        glEnd();
    }
}

static void changement(struct my_nurbs *bs)
{
    if (!(UN_CHAMP_CHANGE(bs) || CREATION(bs)))
        return;

    if (CHAMP_CHANGE(bs, ctrlPts) 
        || CHAMP_CHANGE(bs, pas)
        || CHAMP_CHANGE(bs, degree))
    {
        if (bs->splPts.nb > 0)
            free(bs->splPts.table);
        nurbs(bs);
    }
}

static void calculateSeqNode(struct my_nurbs *bs, int k)
{
    int n = bs->ctrlPts.nb - 1;
    ALLOUER(bs->seqNod.table, n + k + 1);
    bs->seqNod.nb = n + k + 1;

    // Les 0-1 avant et après
    for (int i = 0; i < k; i++)
        bs->seqNod.table[i] = 0;
    for (int i = k; i <= n + k; i++)
        bs->seqNod.table[i] = 1;

    // Composantes au milieu
    for(int i = k; i <= n ; i++)
        bs->seqNod.table[i] = (i - k + 1.0f) / (n - k + 2.0f);
}

static int calculateR(struct my_nurbs *bs, float u)
{ 
    int count = bs->seqNod.nb - 1;
    for (int i = 0; i < count; i++)
    {
        if (u >= bs->seqNod.table[i] && u < bs->seqNod.table[i + 1])
            return i;
    }
    return -1;
}

static void nurbs(struct my_nurbs *bs)
{
    int d = bs->degree; // Degré
    int k = d + 1;      // Ordre

    // Table temporaire pour ne pas modifier
    // Les points de controles de base
    // Les points de la nurbs sont calculés
    // dans cette table.
    Table_quadruplet tempCtrlPts;
    ALLOUER(tempCtrlPts.table, bs->ctrlPts.nb);
    tempCtrlPts.nb = bs->ctrlPts.nb;

    ALLOUER(bs->splPts.table, 1.0f / bs->pas);
    bs->splPts.nb = 1.0f / bs->pas;

    // Sequence Nodale
    calculateSeqNode(bs, k);
    
    // Calcul des points
    for (int h = 0; h < bs->splPts.nb; h++)
    {
        float u = h * bs->pas;
        for (int i = 0; i < bs->ctrlPts.nb; i++)
        {
            tempCtrlPts.table[i].x = bs->ctrlPts.table[i].x * bs->ctrlPts.table[i].h;
            tempCtrlPts.table[i].y = bs->ctrlPts.table[i].y * bs->ctrlPts.table[i].h;
            tempCtrlPts.table[i].z = bs->ctrlPts.table[i].z * bs->ctrlPts.table[i].h;
            tempCtrlPts.table[i].h = bs->ctrlPts.table[i].h;
        }

        int r = calculateR(bs, u);
        for (int j = 1; j <= d; j++)
        {
            for(int i = r - d + j; i <= r; i++)
            {                
                Quadruplet point = tempCtrlPts.table[i];
                Quadruplet nextPoint = tempCtrlPts.table[i - 1];

                Flottant Ui = bs->seqNod.table[i];
                Flottant Uikj = bs->seqNod.table[i + k - j];

                Flottant T1 = (u - Ui) / (Uikj - Ui);
                Flottant T2 = (Uikj - u) / (Uikj - Ui);

                point.x = point.x * T1 + nextPoint.x * T2;
                point.y = point.y * T1 + nextPoint.y * T2;
                point.z = point.z * T1 + nextPoint.z * T2;
                point.h = point.h * T1 + nextPoint.h * T2;

                tempCtrlPts.table[i] = point;
            }
        }
        
        // On stocke les points de spline dans spltPts
        bs->splPts.table[h].x = tempCtrlPts.table[r].x / tempCtrlPts.table[r].h;
        bs->splPts.table[h].y = tempCtrlPts.table[r].y / tempCtrlPts.table[r].h;
        bs->splPts.table[h].z = tempCtrlPts.table[r].z / tempCtrlPts.table[r].h;
    }
}

CLASSE(nurbs, struct my_nurbs,
        CHAMP(ctrlPts, L_table_point P_table_quadruplet Extrait Sauve)
        CHAMP(pas, L_flottant DEFAUT("0.01"))
        CHAMP(degree, LABEL("Degré") L_entier Edite Sauve DEFAUT("2") )
        CHAMP(displayCtrlPts, L_entier Edite DEFAUT("0"))

        CHANGEMENT(changement)
        CHAMP_VIRTUEL(L_affiche_gl(affiche_nurbs))
        EVENEMENT("Shft+B+N")
        MENU("Modélisation Géométrique/Courbe Nurbs"))
