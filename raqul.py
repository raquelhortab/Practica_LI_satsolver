from os import listdir
import os
contenido = ''
archius = listdir(".")
archius = sorted(archius)
for archiu in archius:
	if 'vars' in archiu:
		print archiu
		os.system('./sat <'+str(archiu)+'>r.txt')
		f= open('r.txt','r')
		contenido += str(archiu)+' --> '+ f.read() +'\n'
		f.close()
		os.system('rm r.txt')

f = open('resultats.txt','w')
f.write(contenido)
f.close()
