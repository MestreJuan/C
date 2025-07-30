#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <locale.h>
#include <time.h>

#define TITULO "Drive Thru do Mickey e Donald"
#define TITULO2 "Registrar Pedido"
#define TITULO3 "Bem Vindo Chefe"
#define TITULONF "C�D\tPRODUTO\tQTD\tVALOR"
#define TAMSTR 21
#define MAXPRODUTOS 50
#define MAXFILA 10

FILE *abreArquivo(const char *nomeArq, const char *modo)
{
    FILE *fp = fopen(nomeArq, modo);
    if (!fp)
    {
        printf("Erro ao abrir %s no modo %s\n", nomeArq, modo);
    }
    return fp;
}

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

int totalprodutos, fila[MAXFILA];
int tamFila = 0;

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

void capturaDados(void)
{
    r.codprod = proximoCodigo();
    printf("\nC�digo do produto: %i", r.codprod);
    printf("\nDigite a descri��o do produto: ");
    fflush(stdin);
    fgets(r.descrprod, sizeof(r.descrprod), stdin);
    printf("\nDigite o custo unit�rio do produto: ");
    fflush(stdin);
    scanf("%f", &r.custoprod);
}

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
    getch();
}

void lerDados(void)
{
    FILE *fp = abreArquivo("PRODUTOS.DAT", "rb");
    if (!fp)
    {
        return;
    }
    else
    {
        printf("\nPRODUTOS.DAT aberto para leitura: %p", fp);
        getch();
    }
    while (fread(&r, sizeof(r), 1, fp) == 1)
    {
        printf("\n%3i\t%-21s\tR$%10.2f", r.codprod, r.descrprod, r.custoprod);
    }

    fclose(fp);
    getch();
}

void adicionarFila(int codped)
{
    if (tamFila < MAXFILA)
    {
        fila[tamFila] = codped;
        tamFila++;
        printf("Pedido #%d adicionado � fila de espera.\n", codped);
    }
    else
    {
        printf("\nFila cheia com %d pedidos. Reiniciando fila...\n", MAXFILA);
        tamFila = 0;
        fila[tamFila] = codped;
        tamFila = 1;
        printf("Novo pedido #%d iniciado na fila.\n", codped);
    }
}

void mostrarFila(void)
{
	int i;
    system("cls");
    if (tamFila == 0)
    {
        printf("\nFila de espera atual: (0 pedido(s))\n");
        printf("  (sem pedidos na fila)\n");
    }
    else
    {
        printf("\nFila de espera atual (%d pedido(s)):\n", tamFila);
        for (i = 0; i < tamFila; i++)
        {
            printf(" - Pedido #%d\n", fila[i]);
        }
    }

    printf("\nAperte uma tecla para continuar...");
    system("pause");
}

void mostrarPagamentos(void)
{
    FILE *fp = abreArquivo("PAGAMENTOS.DAT", "rb");
    char buf[20];
    if (!fp)
    {
        printf("\nN�o foi poss�vel abrir PAGAMENTOS.DAT\n");
        getch();
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

char menu2(void)
{
    char opc;
    do
    {
        system("cls");
        printf("\n%s\n", TITULO);
        printf("\n 1. CONSULTAR CARD�PIO");
        printf("\n 2. REGISTRAR PEDIDO");
        printf("\n 0. ENCERRA ");
        printf("\n------------");
        printf("\nSUA ESCOLHA: ");
        fflush(stdin);
        opc = getchar();
    } while (opc < '0' || opc > '2');
    return (opc);
}

int mostrarProdutos(void)
{
    size_t lidos;
    FILE *fp = abreArquivo("PRODUTOS.DAT", "rb");
    if (!fp)
    {
        return 0;
    }
    system("cls");
    printf("\n%s\n", TITULO2);
    while ((lidos = fread(&r, sizeof(r), 1, fp)) == 1)
    {
        printf("\n%3i\t%-21s\tR$%10.2f", r.codprod, r.descrprod, r.custoprod);
    }
    if (ferror(fp))
    {
        printf("\nErro ao ler PRODUTOS.DAT");
        getch();
    }
    fclose(fp);
    return 1;
}

int buscaProdutoSequencial(int cod)
{
    FILE *fp = abreArquivo("PRODUTOS.DAT", "rb");
    if (!fp)
    {
        return 0;
    }

    while (fread(&r, sizeof(r), 1, fp))
    {
        if (r.codprod == cod)
        {
            fclose(fp);
            return (1);
        }
    }
    fclose(fp);
    return (0);
}

void confirmacaoEntrega(char mpag)
{
    const char *VERDE = "\x1b[32m";
    const char *AZUL = "\x1b[34m";
    const char *LILAS = "\x1b[35m";
    const char *RESET = "\x1b[0m";

    const char *cor, *texto;
    switch (mpag)
    {
    case '1':
        cor = VERDE;
        texto = "DINHEIRO";
        break;
    case '2':
        cor = LILAS;
        texto = "CART�O DE D�BITO";
        break;
    case '3':
        cor = LILAS;
        texto = "CART�O DE CR�DITO";
        break;
    case '4':
        cor = AZUL;
        texto = "PIX";
        break;
    default:
        cor = RESET;
        texto = "DESCONHECIDO";
    }

    system("cls");
    printf("%sPEDIDO PAGO COM %s%s\n", cor, texto, RESET);
    system("pause");
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
    getch();
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

int validarCartao(char *numCartao)
{
    int tent = 0;
    printf("\n - - - - - - - PAGAMENTO EM CR�DITO - - - - - - - \n");
    getch();
    while (tent < 3)
    {
        printf("Informe n�mero do cart�o (16 d�gitos): ");
        fflush(stdin);
        scanf("%16s", numCartao);
        if (strlen(numCartao) == 16)
        {
            return 1;
        }
        printf("N�mero inv�lido. Voc� ainda tem %d tentativas.\n", 2 - tent);
        tent++;
    }
    printf("Cartão incorreto 3 vezes. Cancelando pagamento.\n");
    return 0;
}

int validarCartaod(char *numCartao)
{
    int tent = 0;
    printf("\n - - - - - - - PAGAMENTO EM D�BITO - - - - - - - \n");
    getch();
    while (tent < 3)
    {
        printf("Informe n�mero do cart�o (16 d�gitos): ");
        fflush(stdin);
        scanf("%16s", numCartao);
        if (strlen(numCartao) == 16)
        {
            return 1;
        }
        printf("N�mero inv�lido. Voc� ainda tem %d tentativas.\n", 2 - tent);
        tent++;
    }
    printf("Cart�o incorreto 3 vezes. Cancelando pagamento.\n");
    return 0;
}

void censurarCartao(const char *orig, char *censurado)
{
	int i;
    strncpy(censurado, orig, 4);
    for (i = 4; i < 12; i++)
    {
	
        censurado[i] = '*';
    strncpy(censurado + 12, orig + 12, 4);
    censurado[16] = '\0';
	}
}

int proximoCodigoPagamento(void)
{
    return descobreUltimoPedido() + 1;
}

void gravaCartao(void)
{
    FILE *fp = abreArquivo("CARTOES.DAT", "ab");
    if (!fp)
    {
        printf("Erro ao abrir arquivo de cart�es.\n");
        return;
    }

    cart.codped = pag.codped;
    strcpy(cart.cartao, pag.cartaoCensurado);
    cart.total = pag.total;
    fwrite(&cart, sizeof(cart), 1, fp);
    fclose(fp);
}

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

void pagdinheiro(float total)
{
    float valorPago, troco;
    system("cls");
    printf("\n - - - - - - - PAGAMENTO EM DINHEIRO - - - - - - - ");
    printf("\nTotal a Pagar: R$ %.2f\n", total);
    printf("\nInforme o valor entregue: R$ ");
    scanf("%f", &valorPago);
    if (valorPago < total)
    {
        printf("\nValor insuficiente. Cancelando...");
        getch();
        return;
    }
    troco = valorPago - total;
    printf("\nPagamento realizado.\n Troco: R$ %.2f\n", troco);
    system("pause");
}

void pagdebito(void)
{
    char numCartao[17];
    system("cls");
    if (!validarCartaod(numCartao))
    {
        return;
    }
    system("cls");
    printf("Cart�o %4.4s **** **** %4.4s aprovado\n", numCartao, numCartao + 12);
    censurarCartao(numCartao, pag.cartaoCensurado);
    gravaCartao();
}

void pagcredito(void)
{
    char numCartao[17];
    system("cls");
    if (!validarCartao(numCartao))
    {
        return;
    }
    system("cls");
    printf("Cart�o %4.4s **** **** %4.4s aprovado!\n",
           numCartao, numCartao + 12);
    censurarCartao(numCartao, pag.cartaoCensurado);
    gravaCartao();
}

void pagpix(float total)
{
    system("cls");
    printf("\n - - - - - - - PAGAMENTO EM PIX - - - - - - - \n");
    printf("\nTotal a Pagar: R$ %.2f\n", total);
    printf("\nChave pix: 11934605082");
    printf("\nAperte qualquer tecla quando efetuar o pagamento");
    fflush(stdin);
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
        printf("\n2. Cart�o de Cr�dito");
        printf("\n3. Cart�o de D�bito");
        printf("\n4. Pix");
        printf("\nSua escolha:\n");
        fflush(stdin);
        opc = getchar();
    }

    while (opc < '0' || opc > '4');
    return (opc);
}

void gerenciapag(char opc, PEDIDO itens[], int totalItens, float total)
{
    pag.codped = proximoCodigoPagamento();
    pag.datahora = time(NULL);
    pag.total = total;
    pag.mpagamento = opc;

    switch (opc)
    {
    case '1':
        pagdinheiro(total);
        break;

    case '2':
        pagcredito();
        break;

    case '3':
        pagdebito();
        break;

    case '4':
        pagpix(total);
        break;

    default:
        printf("\nOp��o inv�lida");
        return;
    }

    gravaPagamento();
    confirmacaoEntrega(pag.mpagamento);
}

void excluirProduto(void)
{
    REGISTRO produtos[MAXPRODUTOS];
    int total = 0, i, codigo, encontrado = 0;
    FILE *fp;
    system("cls");
    printf("\nDigite o c�digo do produto para excluir: ");
    fflush(stdin);
    scanf("%d", &codigo);
    fp = abreArquivo("PRODUTOS.DAT", "rb");
    if (!fp)
    {
        printf("\nArquivo n�o encontrado");
        return;
    }
    while (fread(&produtos[total], sizeof(r), 1, fp) == 1 && total < MAXPRODUTOS)
    {
        total++;
    }
    fclose(fp);

    for (i = 0; i < total; i++)
    {
        if (produtos[i].codprod == codigo)
        {
            encontrado = 1;
            break;
        }
    }

    if (!encontrado)
    {
        printf("C�digo n�o encontrado");
        return;
    }

    for (; i < total - 1; i++)
    {
        produtos[i] = produtos[i + 1];
    }
    total--;

    fp = abreArquivo("PRODUTOS.DAT", "wb");
    if (!fp)
    {
        printf("\nErro ao abrir o arquivo");
        return;
    }
    for (i = 0; i < total; i++)
    {
        fwrite(&produtos[i], sizeof(r), 1, fp);
    }
    fclose(fp);
    printf("\nC�digo removido");
}

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

void registrarpedido(void)
{
    PEDIDO itens[MAXPRODUTOS];
    int totalItens = 0, novoCodigo;
    int codigo, flag, quantidade;
    float total = 0.0f;
    char opc, opcpag;

    if (mostrarProdutos() == 0)
    {
        exit(0);
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
                getche();
            }
            else
                flag = 1;
        } while (flag == 0);

        do
        {
            printf("Quantidade de '%s': ", r.descrprod);
            fflush(stdin);
            scanf("%d", &quantidade);
            if (quantidade <= 0)
            {
                printf("Quantidade invalida!!!!");
                getch();
            }
        } while (quantidade <= 0);

        itens[totalItens].codprod = r.codprod;
        strcpy(itens[totalItens].descrprod, r.descrprod);
        itens[totalItens].quantidade = quantidade;
        itens[totalItens].subtotal = quantidade * r.custoprod;
        total += itens[totalItens].subtotal;
        totalItens++;

        printf("\nTotal parcial = R$ %10.2f", total);

        printf("\nDeseja mais alguma coisa? [aperte 0 para ir ao pagamento] ");

        fflush(stdin);
        opc = getch();
        if (opc == '0')
        {

            if (total > 300.0f)
            {
                novoCodigo = proximoCodigoPagamento();
                pag.codped = novoCodigo;
                adicionarFila(pag.codped);
                opcpag = menupag();
                gerenciapag(opcpag, itens, totalItens, total);
            }
            else
            {
                opcpag = menupag();
                gerenciapag(opcpag, itens, totalItens, total);
                            gerarComanda(itens, totalItens, total);
            exibirComanda();
            }
        }
    } while (opc != '0');
}

void gerencia2(char opc)
{
    switch (opc)
    {
    case '0':
        exit(0);
        break;
    case '1':
    {
        lerDados();
        break;
    }
    break;
    case '2':
        registrarpedido();
        break;
    }
}

int senha(void)
{
    char s1, s2, s3, s4;
    printf("Digite a senha de administrador:\n");
    s1 = getch();
    printf("*");
    s2 = getch();
    printf("*");
    s3 = getch();
    printf("*");
    s4 = getch();
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

char menu3(void)
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
        exit(0);
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
        mostrarFila();
        break;
    }
    case'5':
    {
        mostrarPagamentos();
        break;
    }
    }
}

int gerencia1(char opc)
{
    switch (opc)
    {
    case '0':
        return 0;
        break;
    case '1':
    {
        return 1;
    }
    break;
    case '2':
        return 2;
        break;
    }
}

char menu1(void)
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
        opc = getchar();
    } while (opc < '0' || opc > '2');
    return (opc);
}

int main()
{
    char op1, op2, op3;
    int trava, a;
    setlocale(LC_ALL, "");

    while (1)
    {
        do
        {
            op1 = menu1();
        } while (op1 < '0' || op1 > '2');

        a = gerencia1(op1);

        if (a == 0)
        {
            break;
        }
        else if (a == 1)
        {
            do
            {
                system("cls");
                fflush(stdin);
                op2 = menu2();
                gerencia2(op2);
            } while (op2 != '0');
        }
        else if (a == 2)
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
                    op3 = menu3();
                    gerencia3(op3);
                } while (op3 != '0');
            }
            else
            {
                printf("\nSenha incorreta. Retornando ao menu principal.\n\n");
            }
        }
    }

    printf("Programa encerrado.\n");
    fflush(stdin);
    getch();
    return 0;
}
