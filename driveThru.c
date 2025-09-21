#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>

//win ou linux

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>

    #define getch_customizado() getch()

    typedef HANDLE thread_t;

    int thread_create(thread_t *thread, void *(*inicio) (void *), void *arg)
    {
        *thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)inicio, arg, 0, NULL);
        return (*thread != NULL) ? 0 : 1;
    }



#else 
    #include <termios.h>
    #include <unistd.h>
    #include <pthread.h>

    int getch_customizado(void)
    {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }

#endif

//win ou linux

#define TITULO "Drive Thru do Mickey e Donald"
#define TITULO2 "Registrar Pedido"
#define TITULO3 "Bem Vindo Chefe"
#define TITULONF "COD\tPRODUTO\t\tQTD\tVALOR"
#define TAMSTR 21
#define VERDE "\x1b[32m"
#define AZUL "\x1b[34m"
#define ROXO "\x1b[35m"
#define NORMAL "\x1b[0m"

void registrarpedido(void);
int descobreUltimoPedido(void);
void mostrarPagamentos(void);
int contarProdutos(void);

typedef struct
{
    int codped;
    char cartao[17];
    float total;
} CARTAO;
CARTAO cart;

typedef struct
{
    int codped;
    time_t datahora;
    float total;
    char mpagamento;
    char cartaoCensurado[17];
} PAGAMENTO;
PAGAMENTO pag;

typedef struct
{
    int codprod;
    char descrprod[TAMSTR];
    float custoprod;
    int quantidade;
    float subtotal;
} PEDIDO;
PEDIDO ped;

typedef struct
{
    int codprod;
    char descrprod[TAMSTR];
    float custoprod;
} REGISTRO;
REGISTRO r;

//ponteiras

FILE *abreArquivo(const char *nomeArq, const char *modo)
{
    FILE *fp = fopen(nomeArq, modo);
    if (!fp)
    {
        printf("Erro ao abrir %s no modo %s\n", nomeArq, modo);
    }
    return fp;
}

REGISTRO* memoriaProdutos(int *quantidadeTotal)
{
    int numProdutos;

    numProdutos = contarProdutos();
    if (numProdutos == 0) 
    {
        *quantidadeTotal = 0;
        return NULL; 
    }

    REGISTRO *vetorDeProdutos = (REGISTRO*) malloc(numProdutos * sizeof(REGISTRO));
    if (vetorDeProdutos == NULL) 
    {
        printf("Falha ao alocar memoria\n");
        *quantidadeTotal = 0;
        return NULL;
    }

    FILE *fp = abreArquivo("PRODUTOS.DAT", "rb");
    if (!fp) 
    {
        free(vetorDeProdutos);
        *quantidadeTotal = 0;
        return NULL;
    }

    fread(vetorDeProdutos, sizeof(REGISTRO), numProdutos, fp);
    *quantidadeTotal = numProdutos;

    fclose(fp);

    return vetorDeProdutos;
}

//ponteiras

//Códigos

int proximoCodigo(void)
{
    int maxCod = 0;
    FILE *fp = abreArquivo("PRODUTOS.DAT", "rb");
    if (!fp)
    {
        return 1;
    }

    while (fread(&r, sizeof(r), 1, fp) == 1)
    {
        if (r.codprod > maxCod)
        {
            maxCod = r.codprod;
        }
    }
    fclose(fp);
    return maxCod + 1;
}

//Códigos 

//Produtos

int contarProdutos(void)
{
    int numeroProdutos;
    long tamanhoTotal;

    FILE *fp = abreArquivo("PRODUTOS.DAT", "rb");
    if (!fp) 
    {
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    tamanhoTotal = ftell(fp);
    numeroProdutos = tamanhoTotal / sizeof(REGISTRO);

    fclose(fp);

    return numeroProdutos;
}

void capturaDados(void)
{
    r.codprod = proximoCodigo();
    printf("\nCódigo do produto: %i", r.codprod);
    printf("\nDigite a descrição do produto: ");
    fflush(stdin);
    fgets(r.descrprod, sizeof(r.descrprod), stdin);
    printf("\nDigite o custo unitário do produto: ");
    fflush(stdin);
    scanf("%f", &r.custoprod);
}  //pegar

void gravarDados(void)
{
    FILE *fp = abreArquivo("PRODUTOS.DAT", "ab");
    if (!fp)
    {
        return;
    }

    fwrite(&r, sizeof(r), 1, fp);
    fclose(fp);
    printf("\nDados gravados com sucesso");
    printf("\nAperte enter para continuar");
    getchar();
}  //gravar

void lerDados(void)
{
    int total = 0, i;
    REGISTRO *produtos = memoriaProdutos(&total);
    if (produtos == NULL || total == 0)
    {
        printf("\nNenhum produto cadastrado.\n");
        printf("Aperte enter para continuar...");
        getchar();
        free(produtos);
        return;
    }

    for (int i = 0; i < total; i++) 
    {
        printf("\n%3i\t%-21s\tR$%10.2f", produtos[i].codprod, produtos[i].descrprod, produtos[i].custoprod);
    }

    printf("\nAperte enter para continuar");
    getchar();

    free(produtos);
}  //ler

int mostrarProdutos(void)
{
    int total = 0, i;
    REGISTRO *produtos = memoriaProdutos(&total);
    if (produtos == NULL || total == 0) 
    {
        printf("\nNenhum produto cadastrado.\n");
        printf("Aperte enter para continuar...");
        getchar();
        free(produtos);
        return 0;
    }

    for (i = 0; i < total; i++) 
    {
        printf("\n%3i\t%-21s\tR$%10.2f", produtos[i].codprod, produtos[i].descrprod, produtos[i].custoprod);
    }

    printf("\nAperte enter para continuar");
    getchar();

    free(produtos);

    return 1;      
} //mostrar

int buscaProdutoSequencial(int cod)
{
    int total = 0, i;
    REGISTRO *produtos = memoriaProdutos(&total); 
    if (produtos == NULL || total == 0) 
    {
        free(produtos);
        return 0;
    }

    for (i = 0; i < total; i++) 
    {
        if (produtos[i].codprod == cod)
        {
            r = produtos[i];
            free(produtos);
            return (1);
        }
    }

    free(produtos);

    return (0);
} //buscar

void excluirProduto(void)
{
    int totalDeProdutos = 0, i, codigo, encontrado = 0;;

    REGISTRO *produtos = memoriaProdutos(&totalDeProdutos);
    if (produtos == NULL || totalDeProdutos == 0) 
    {
        printf("\nNao ha produtos cadastrados para excluir.\n");
        printf("Aperte enter para continuar...");
        getchar();
        return; 
    }

    system("cls");
    printf("\nDigite o código do produto para excluir: ");
    fflush(stdin);
    scanf("%d", &codigo);

    for (i = 0; i < totalDeProdutos; i++)
    {
        if (produtos[i].codprod == codigo) 
        {
            encontrado = 1;
            break;
        }
    }

    if (!encontrado) 
    {
        printf("Código não encontrado\n");
    }

    else 
    {
        for (; i < totalDeProdutos - 1; i++) {
            produtos[i] = produtos[i + 1];
        }
        totalDeProdutos--;

        FILE *fp = abreArquivo("PRODUTOS.DAT", "wb");
        if (fp) 
        {
            fwrite(produtos, sizeof(REGISTRO), totalDeProdutos, fp);
            fclose(fp);
            printf("\nCódigo removido com sucesso!\n");
        } 
        
        else 
        {
            printf("\nErro ao reescrever o arquivo de produtos!\n");
        }
    }

    free(produtos);
    
    printf("\nAperte enter para continuar");
    getchar();
} //excluir

//Produtos 

//Menus

int menuInicial(void)
{
    char opc;
    do
    {
        system("cls");
        printf("\n%s\n", TITULO);
        printf("\n 1. ENTRAR COMO USUÁRIO");
        printf("\n 2. ENTRAR COMO ADMINISTRADOR");
        printf("\n 0. ENCERRA ");
        printf("\n------------");
        printf("\nSUA ESCOLHA: ");
        fflush(stdin);
        scanf("%c", &opc);
    } while (opc < '0' || opc > '2');
    return (opc);
}

char gerenciaInicial(char opc)
{
    switch (opc)
    {
        case '0':
        {
            return '0';
            break;
        }

        case '1':
        {
            return '1';
            break;
        }

        case '2':
        {
            return '2';
            break;
        }
    }
}

char menuUsuario(void)
{
    char opc;
    do
    {
        system("cls");
        printf("\n%s\n", TITULO);
        printf("\n 1. CONSULTAR CARDÁPIO");
        printf("\n 2. REGISTRAR PEDIDO");
        printf("\n 0. ENCERRA ");
        printf("\n------------");
        printf("\nSUA ESCOLHA: ");
        fflush(stdin);
        opc = getchar();
    } while (opc < '0' || opc > '2');
    return (opc);
}

void gerencia2(char opc)
{
    switch (opc)
    {
        case '0':
        {

            break;
        }
    
        case '1':
        {
            lerDados();
            break;
        }
    
        case '2':
        {
            registrarpedido();
            break;
        }
    }
}

char menuAdm(void)
{
    char opc;
    do
    {
        system("cls");
        printf("\n%s\n", TITULO3);
        printf("\n 1. CADASTRAR NOVO PRODUTO");
        printf("\n 2. CONSULTAR PRODUTOS");
        printf("\n 3. EXCLUIR PRODUTO");
        printf("\n 4. VER FILA");
        printf("\n 5. VER PAGAMENTOS");
        printf("\n 0. ENCERRA ");
        printf("\n------------");
        printf("\nSUA ESCOLHA: ");
        fflush(stdin);
        opc = getchar();
    } while (opc < '0' || opc > '5');

    return (opc);
}

int gerencia3(char opc)
{
    switch (opc)
    {
    case '0':
    {
        
        break;
    }
    case '1':
    {
        capturaDados();
        gravarDados();
        break;
    }
    break;
    case '2':
    {
        lerDados();
        break;
    }
    case '3':
    {
        excluirProduto();
        break;
    }
    case '4':
    {
        //mostrarFila();
        break;
    }
    case'5':
    {
        mostrarPagamentos();
        break;
    }
    }
}

//Menus

//Confirmação

void confirmacaoEntrega(char mpag)
{
    const char *cor, *texto;
    switch (mpag)
    {
    case '1':
        cor = VERDE;
        texto = "DINHEIRO";
        break;
    case '2':
        cor = ROXO;
        texto = "CARTÃO DE DÉBITO";
        break;
    case '3':
        cor = ROXO;
        texto = "CARTÃO DE CRÉDITO";
        break;
    case '4':
        cor = AZUL;
        texto = "PIX";
        break;
    default:
        cor = NORMAL;
        texto = "DESCONHECIDO";
    }

    system("cls");
    printf("%sPEDIDO PAGO COM %s%s\n", cor, texto, NORMAL);
    system("pause");
}

//Connfirmação

//Pagamentos

//void pagpix(float total);
//void pagCartao(void);
//void pagDinheiro(float total);

void gravaPagamento(void)
{
    FILE *fp = abreArquivo("PAGAMENTOS.DAT", "ab");
    if (!fp)
    {
        printf("\nErro ao abrir PAGAMENTOS.DAT");
        return;
    }
    fwrite(&pag, sizeof(pag), 1, fp);
    fclose(fp);
}

void mostrarPagamentos(void)
{
    FILE *fp = abreArquivo("PAGAMENTOS.DAT", "rb");
    char buf[20];
    if (!fp)
    {
        printf("\nNão foi possível abrir PAGAMENTOS.DAT\n");
        printf("\nAperte enter para continuar");
        getchar();
        return;
    }

    system("cls");
    printf("C�D  DATA/HORA           TOTAL     MPAG  CART�O\n");
    printf("---- ------------------- --------- ----- ----------------\n");

    while (fread(&pag, sizeof(pag), 1, fp) == 1)
    {
        struct tm *tm_info = localtime(&pag.datahora);
        strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", tm_info);

        printf("%4d  %-19s R$%8.2f   %c   %s\n",
               pag.codped,
               buf,
               pag.total,
               pag.mpagamento,
               pag.cartaoCensurado[0] ? pag.cartaoCensurado : "-");
    }

    fclose(fp);
    printf("\nAperte qualquer tecla para voltar...");
    system("pause");
}

char menupag(void)
{
    char opc;

    do
    {
        system("cls");
        printf("Escolha sua forma de pagamento:");
        printf("\n1. Dinheiro");
        printf("\n2. Cartão de Crédito");
        printf("\n3. Cartão de Débito");
        printf("\n4. Pix");
        printf("\nSua escolha:\n");
        fflush(stdin);
        scanf("%c", &opc);
    } while (opc < '0' || opc > '4');

    return (opc);
}

void gerenciapag(char opc, PEDIDO itens[], int totalItens, float total)
{
    pag.codped = descobreUltimoPedido() + 1;
    pag.datahora = time(NULL);
    pag.total = total;
    pag.mpagamento = opc;

    switch (opc)
    {
        case '1':
        {
            //pagDinheiro(total);
            break;
        }

        case '2':
        {
            //pagCredito();
            break;
        }

        case '3':
        {
            //pagDebito();
            break;
        }

        case '4':
        {
            //pagPix(total);
            break;
        }

        default:
        {
            printf("\nOpção inválida");
            return;
        }
    }

    gravaPagamento();
    confirmacaoEntrega(pag.mpagamento);
}

//Pagamentos

//Pedido

void gerarComanda(PEDIDO itens[], int totalItens, float total)
{
    int i;
    FILE *fc = abreArquivo("COMANDA.TXT", "w");
    if (!fc)
    {
        printf("\nErro ao abrir a comanda");
        return;
    }
    fprintf(fc, " - - - - - - - COMANDA - - - - - - - \n\n");
    fprintf(fc, "%s\n\n", TITULONF);

    for (i = 0; i < totalItens; i++)
    {
        fprintf(fc, "%3i\t%-21s\t%3i\tR$%8.2f\n", itens[i].codprod, itens[i].descrprod, itens[i].quantidade, itens[i].subtotal);
    }
    fprintf(fc, "\nTotal a Pagar:   R$ %.2f\n", total);
    fclose(fc);
}

void exibirComanda(void)
{
    FILE *fp = fopen("COMANDA.TXT", "r");
    char linha[128];

    if (fp == NULL)
    {
        printf("\nErro ao abrir COMANDA.TXT\n");
        return;
    }

    system("cls");

    while (fgets(linha, sizeof(linha), fp) != NULL)
    {
        printf("%s", linha);
    }

    fclose(fp);

    printf("\nAperte qualquer tecla para continuar");
    getchar();
}


int descobreUltimoPedido(void)
{
    FILE *Pag = abreArquivo("PAGAMENTOS.DAT", "rb");
    if (!Pag)
    {
        return 0;
    }

    int ultimo = 0;
    while (fread(&pag, sizeof(pag), 1, Pag) == 1)
    {
        ultimo = pag.codped;
    }

    fclose(Pag);
    return ultimo;
}

void registrarpedido(void)
{
    PEDIDO *itens = NULL;
    int totalItens = 0, novoCodigo;
    int codigo, flag, quantidade, novoIndice;
    float total = 0.0f;
    char opc, opcpag;

    if (mostrarProdutos() == 0)
    {
        printf("\nErro ao mostrar produtos");
        return;
    }

    do
    {
        do
        {
            flag = 0;
            printf("\nDigite o C�digo do Produto: ");
            fflush(stdin);
            scanf("%i", &codigo);
            if (buscaProdutoSequencial(codigo) == 0)
            {
                printf("\nCodigo nao Cadastrado\n");
                getchar();
            }
            else
            {
                flag = 1;
            }

        } while (flag == 0);

        do 
        {
            printf("Quantidade de '%s': ", r.descrprod);
            fflush(stdin);
            scanf(" %d", &quantidade);
            if (quantidade <= 0)
            {
                printf("Quantidade invalida!!!!");
                getchar();
            }
        } while (quantidade <= 0);

        totalItens++;

        PEDIDO *temp = realloc(itens, totalItens * sizeof(PEDIDO));
        if (temp == NULL)
        {
            printf("Falha ao alocar memoria\n");
            free(itens); 
            return; 
        }
        itens = temp;

        novoIndice = totalItens - 1;

        itens[novoIndice].codprod = r.codprod;
        strcpy(itens[novoIndice].descrprod, r.descrprod);
        itens[novoIndice].quantidade = quantidade;
        itens[novoIndice].subtotal = quantidade * r.custoprod;
        total += itens[novoIndice].subtotal;

        printf("\nTotal parcial = R$ %10.2f", total);

        printf("\nDigite qualquer tecla para adicionar mais algum item ou APERTE 0 para encerrar");

        fflush(stdin);
        scanf(" %c", &opc);

    } while (opc != '0');

    opcpag = menupag();
    gerenciapag(opcpag, itens, totalItens, total);
    gerarComanda(itens, totalItens, total);
    exibirComanda();

    free(itens);

}

//Pedido

//Senha

int senha(void)
{
    char s1, s2, s3, s4;
    printf("Digite a senha de administrador:\n");
    fflush(stdin);
    s1 = getch_customizado();
    printf("*");
    fflush(stdin);
    s2 = getch_customizado();
    printf("*");
    fflush(stdin);
    s3 = getch_customizado();
    printf("*");
    fflush(stdin);
    s4 = getch_customizado();
    printf("*");
    if (s1 == '1' && s2 == '2' && s3 == '3' && s4 == '4')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//Senha

int main()
{
    setlocale(LC_ALL, "Portuguese");

    char opc, op1, op2, op3;
    int trava;



    do 
    {
        op1 = menuInicial();

        opc = gerenciaInicial(op1);

        switch(opc)
        {
            //finaliza
            case '0':
            {
                printf("\nEncerrando Programa...");
                return 0;
                break;
            }

            //cliente 
            case '1': 
            {
                do
                {
                    system("cls");
                    fflush(stdin);
                    op2 = menuUsuario(); 
                    gerencia2(op2);

                } while (op2 != '0');
                break;
            }

            //admin
            case '2': 
            {
                system("cls");
                fflush(stdin);
                trava = senha();

                if (trava == 1)
                {
                    do 
                    {
                        system("cls");
                        fflush(stdin);
                        op3 = menuAdm(); 
                        gerencia3(op3);

                    } while (op3 != '0');
                    break;
                }
                else 
                {
                    printf("\nSenha incorreta. Retornando ao menu principal.\n\n");
                    break;
                }
            }
        } 

    } while (op1 != '0');

    printf("\nEncerrando Programa");

    return 0;
}