import random
import os
from decimal import Decimal

def reading_file (path):

    ''' 
    Recebe como parâmetro o caminho do arquivo que deseja ler.
    A função tentará, ler o arquivo do caminho fornecido.
    Retornando uma matriz com o que foi lido do arquivo·
    '''

    try:
        with open (path, "r") as file:
            return file.readlines ()
    except:
        print ("ERROR: reading_file")

def devices_matrix (matrix_read, length):

    '''
    Recebe como parâmetro uma matriz que contem as linhas lidas do arquivo
    e também a quantidade de linhas que possuia o arquivo.
    A função, gera uma nova matriz com os dados necessários para os estudos.
    Retorna a nova matriz com os dados necessários para os estudos.
    '''    

    if (length != 1000): # se for menor que 1000 manda sortear.
        aux = set()
    
        while len(aux) < length:
            aux.add (int (random.uniform (1, 1001)))  
        
        numbers = list(aux)

    else: # caso que não precisa sortear, usando todos da lista de dispositivos.
        numbers = []
        for i in range (1,length + 1):
            numbers.append (i)
    
    services = new_services_file (5)

    new_matrix = []

    for i in range (len (numbers)):

        new_matrix.append (matrix_read[numbers[i]].split ()) # longitude e latitude
        new_matrix[i][0] = i + 1 # enumera as linhas

        j = int(random.uniform (1,6)) # sorteia o serviço
        
        line = services[j].split ()
        
        new_matrix[i][3] =   (line[1]) # pcc
        new_matrix[i].append (line[2]) # pcn
        new_matrix[i].append (line[3]) # mem
        new_matrix[i].append (line[4]) # st
        new_matrix[i].append (line[5]) # bw
        new_matrix[i].append (line[6]) # sd
        new_matrix[i].append (line[7]) # cost
        new_matrix[i].append (line[0]) # service
        new_matrix[i].append (line[8]) # task
        
    return new_matrix
    
def ec_servers_matrix (matrix_read):

    '''
    Recebe como parâmetro uma matriz que contem as linhas lidas do arquivo.
    A função, gera uma nova matriz com os dados necessários para os estudos.
    Retorna a nova matriz com os dados necessários para os estudos.
    '''    

    try:

        number = matrix_read[0].strip ()
        loop = int (number)

        new_matrix = []

        for i in range (loop):

            server = random_limiter ("ec")

            new_matrix.append    (matrix_read[i + 1].split ()) # latitude e longitude
            new_matrix[i][0] =    i + 1                        # enumera as linhas
            new_matrix[i][3] =    server[0]                    # pcc
            new_matrix[i][4] =    server[1]                    # pcn
            new_matrix[i].append (server[2])                   # mem
            new_matrix[i].append (server[3])                   # st
            new_matrix[i].append (server[4])                   # bw
            new_matrix[i].append (server[5])                   # cost
            new_matrix[i].append (server[6])                   # tp

    except:
        print ("ERROR: ec_servers_matrix")
    finally:    
        return new_matrix

def cc_servers_matrix (matrix_read):

    '''
    Recebe como parâmetro uma matriz que contem as linhas lidas do arquivo.
    A função, gera uma nova matriz com os dados necessários para os estudos.
    Retorna a nova matriz com os dados necessários para os estudos.
    '''    

    try:

        new_matrix = []
        
        for i in matrix_read:
            new_matrix.append (i.split ())

        for i in range (1, len (new_matrix)):
            new_matrix[i][-1] = round (12.5 / float (new_matrix[i][3]), 5)
            new_matrix[i][3] = float (new_matrix[i][3]) * int (new_matrix[i][4])
            new_matrix[i][7] = 0
            
    except:
        print ("ERROR: cc_servers_matrix")
    finally:    
        return new_matrix

def random_limiter (category):

    '''
    Recebe como parâmetro a categoria que será sorteado os valores para estudos.
    A função com base na categoria, sorteia valores para cada um dos estudos.
    Retorna o resultado do sorteio.
    '''
    
    result = []
    
    if category == "service":
        task    = random.randint (1, 4)
        pcc     = 0
        pcn     = int(random.uniform (1, 5))
        memory  = 0
        storage = 0 
        bw      = 0
        sd      = random.uniform (0.00484, 12) # Megabit
        cost    = 1
   
        for i in range (task):
            pcc     += random.uniform (0.001, 2.5)
            memory  += random.uniform (0.001, 2.5)
            storage += random.uniform (0.001, 15)

        '''
        cost = 0
        if   (pcn == 1):
            cost += 3
        elif (pcn == 2):
            cost += 5
        elif (pcn == 3):
            cost += 7
        elif (pcn == 4):
            cost += 9
        else:
            cost += -1

        if   (pcc >= 0 and pcc < 2.5):
            cost += Decimal('0.3')
        elif (pcc >= 2.5 and pcc < 5):
            cost += Decimal('0.5')
        elif (pcc >= 5 and pcc < 7.5):
            cost += Decimal('0.7')
        elif (pcc >= 7.5 and pcc < 10):
            cost += Decimal('0.9')
        else:
            cost += -1
               
        if   (memory > 0.0 and memory <= 2.5):
            cost += Decimal('0.03')
        elif (memory > 2.5 and memory <= 5.0):
            cost += Decimal('0.05')
        elif (memory > 5.0 and memory <= 7.5):
            cost += Decimal('0.07')
        elif (memory > 7.5 and memory <= 10.0):
            cost += Decimal('0.09')
        else:
            cost += -1

        if   (storage > 0.00 and storage <= 15.0):
            cost += Decimal('0.0025')
        elif (storage > 15.0 and storage <= 30.0):
            cost += Decimal('0.0050')
        elif (storage > 30.0 and storage <= 45.0):
            cost += Decimal('0.0075')
        elif (storage > 45.0 and storage <= 60.0):
            cost += Decimal('0.01')
        else:
            cost += -1
        '''
        
        result.append (round (pcc, 5))
        result.append (pcn)
        result.append (round (memory, 5))
        result.append (round (storage, 5))
        result.append (bw)
        result.append (round (sd, 5))
        result.append (cost)
        result.append (task)
        
    elif category == "ec":

        raffle  = random.randint (1, 5)
        memory  = random.uniform (0.001, 125)
        storage = random.uniform (0.001, 1000)
        bw      = 0
        tp      = 12.5
        
        if   raffle == 1:
            pcn  = 2
            pcc  = 1.6
            cost = 0.00085
            tp  /= pcc

        elif raffle == 2:
            pcn  = 4
            pcc  = 2.3
            cost = 0.00097
            tp  /= pcc

        elif raffle == 3:
            pcn  = 6
            pcc  = 2.9
            cost = 0.00121
            tp  /= pcc

        elif raffle == 4:
            pcn  = 8
            pcc  = 3
            cost = 0.00138
            tp  /= pcc

        elif raffle == 5:
            pcn  = 10
            pcc  = 3
            cost = 0.00153
            tp  /= pcc

        else:
            pcn  = -1
            pcc  = -1
            cost = -1
            tp   = -1

        result.append (pcc * pcn)
        result.append (pcn)
        result.append (round (memory, 5))
        result.append (round (storage, 5))
        result.append (bw)
        result.append (cost)
        result.append (round (tp, 5))

    return result

def new_services_file (length):

    '''
    Recebe como parâmetro o tamanho do vetor que será usado para gerar o arquivo.
    A função gera um arquivo com as informações de serviços que serão usadas nos estudos.
    Retornando a matriz de dados de serviço para uso nos dispositos.
    '''

    try:

        matrix = []
        path = "data/services_{}.txt".format (length)

        if (not os.path.exists (path)):

            for i in range (length):

                service = random_limiter ("service")

                line = []
                line.append (i + 1)

                for j in service:
                    line.append (j)

                matrix.append (line)

            with open (path, "w") as file:
                
                file.write ("service pcc pcn mem st bw sd cost task")
            
                for i in matrix:
                    
                    text = "\n"

                    for j in i:
                        text += str (j) + " "

                    file.write (text)

            file.close ()

    except:
        print ("ERROR: new_services_file")

    finally:
        matrix = reading_file (path)
        return matrix
    
def new_devices_file (length):

    '''
    Recebe como parâmetro o tamanho do vetor que será usado para gerar o arquivo.
    A função gera um arquivo com as informações de dispositivos que serão usadas nos estudos.
    '''
    
    try:
        read = reading_file ("data/sensores1000.txt")
        matrix = devices_matrix (read, length)
                 
        path = "devices"
        
        if (not os.path.exists (path)):
            os.mkdir (path)

        with open (path + "/" + "{}_devices.txt".format (length), "w") as file:
            file.write ("{} lat long pcc pcn mem st bw sd cost service task".format (length))
            
            for i in matrix:
                text = "\n"
                for j in i:
                    text += str (j) + " "
                file.write (text)

    except:
        print ("ERROR: new_devices_file")

def new_cc_servers_file (length):

    ''' 
    Recebe como parâmetro o tamanho do vetor que será usado para ler e gerar o arquivo.
    A função gera um arquivo com as informações de servidor que serão usadas nos estudos.
    '''
    
    try:

        read_file = reading_file ("data/CC_{}.txt".format (length))
        matrix = cc_servers_matrix (read_file)

        path = "servers"
        if (not os.path.exists (path)):
            os.mkdir (path)
        
        with open (path + "/" + ("CC_{}_servers.txt".format (length)), "w") as file:
            
            for i in matrix:
                text = ""
                for j in i:
                    text += str (j) + " "
                text += "\n"
                file.write (text)

    except:
        print("ERROR: new_cc_servers_file")
    finally:
        file.close () 

def new_ec_servers_file (length):

    ''' 
    Recebe como parâmetro o tamanho do vetor que será usado para ler e gerar o arquivo.
    A função gera um arquivo com as informações de servidor que serão usadas nos estudos.
    '''
    
    try:

        read_file = reading_file ("data/EC_{}.txt".format (length))
        matrix = ec_servers_matrix (read_file)
        
        path = "servers"
        if (not os.path.exists (path)):
            os.mkdir (path)
        
        with open (path + "/" + ("EC_{}_servers.txt".format (length)), "w") as file:
            file.write ("{} LAT LONG PCC PCN MEM ST BW COST TP".format (length))
            
            for i in matrix:
                text = "\n"
                for j in i:
                    text += str (j) + " "
                file.write (text)
    except:
        print("ERROR: new_ec_servers_file")
    finally:
        file.close ()

'''**************************************************MAIN****************************************'''
def main ():

    try:
        new_cc_servers_file (5)
        new_ec_servers_file (10)
        new_ec_servers_file (50)
        new_ec_servers_file (100)    

    finally:
        print("*****"*10)
        print("\nArquivos de servidores pronto\n")
        print("*****"*10)
    
    try:
        length = 50

        while (length <= 1000):

            new_devices_file (length)
            length += 50

    finally:

        print("*****"*10)
        print("\nArquivos de dispositivos pronto\n")
        print("*****"*10)
    
main ()
