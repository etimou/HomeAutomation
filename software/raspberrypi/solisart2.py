import requests
import urllib3
import time
import datetime
import xmltodict
import json
import base64
from paho.mqtt import client as mqtt_client
import random

broker = 'secret'
port = secret
topic = "/secret"
client_id = f'python-mqtt-{random.randint(0, 1000)}'


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print("Failed to connect, return code %d\n", rc)



urllib3.disable_warnings()

session = requests.Session()

def loadCookie():
	f = open("./solisartcookie", 'r')
	phpsessid = f.read().rstrip()
	session.cookies.update({'PHPSESSID': phpsessid})
	f.close()

def saveCookie():
	f = open("./solisartcookie", 'w')
	f.write(session.cookies.get_dict()['PHPSESSID'])
	f.close()
	

def submit_form_and_get_cookie(url, form_data):
    # Crée une session pour gérer les cookies

   
    # Soumettre le formulaire avec une requête POST
    response = session.post(url, data=form_data, verify=False)
       
    # Vérifie si la requête a réussi
    if response.status_code == 200:
        print("Formulaire soumis avec succès!")
        # Récupère les cookies de la session
        cookies = session.cookies.get_dict()
        return cookies
    else:
        print(f"Échec de la soumission du formulaire : {response.status_code}")
        return None

def submit_form_and_get_cookie_solisart():
	url = "secret"
	form_data = {
	    "id":"secret",
	    "pass":"secret",
	    "ihm":"admin",
	    "connexion":"Se+connecter",
	}
	submit_form_and_get_cookie(url, form_data)
	saveCookie()


def lecture_valeurs_donnees():
	url = "https://my.solisart.fr/admin/divers/ajax/lecture_valeurs_donnees.php"
	form_data = {
		"id":"secret",
		"heure":str(int(time.time())),
		"periode":"60",
	}

	valeurs_donnees = []
	try:
		response = session.post(url, data=form_data, verify=False, timeout=4)
		response.raise_for_status()
		
	except requests.exceptions.RequestException as err:
		print ("OOps: Something Else",err)
	except requests.exceptions.HTTPError as errh:
		print ("Http Error:",errh)
	except requests.exceptions.ConnectionError as errc:
		print ("Error Connecting:",errc)
	except requests.exceptions.Timeout as errt:
		print ("Timeout Error:",errt)  	

	valeurs_donnees = xmltodict.parse(response.text)

	status = valeurs_donnees['valeurs']['@statut']
	print(status)
	
	if (status == "echec"):
		return 0

	elif (status == "succes"):
		payload={}

		if ('valeur' in valeurs_donnees['valeurs'].keys()):

			for donnee in valeurs_donnees['valeurs']['valeur']:
				donnee['@valeur']=base64.b64decode(donnee['@valeur'])
				#print(donnee)
				if (donnee['@donnee'] == '584'):
                                        payload['T1'] = float(donnee['@valeur'][:-3])
				if (donnee['@donnee'] == '585'):
                                        payload['T2'] = float(donnee['@valeur'][:-3])
				if (donnee['@donnee'] == '586'):
                                        payload['T3'] = float(donnee['@valeur'][:-3])
				if (donnee['@donnee'] == '587'):
                                        payload['T4'] = float(donnee['@valeur'][:-3])
				if (donnee['@donnee'] == '588'):
                                        payload['T5'] = float(donnee['@valeur'][:-3])
				if (donnee['@donnee'] == '589'):
                                        payload['T6'] = float(donnee['@valeur'][:-3])
				if (donnee['@donnee'] == '590'):
                                        payload['T7'] = float(donnee['@valeur'][:-3])
				if (donnee['@donnee'] == '591'):
                                        payload['T8'] = float(donnee['@valeur'][:-3])
				if (donnee['@donnee'] == '592'):
                                        payload['T9'] = float(donnee['@valeur'][:-3])
				if (donnee['@donnee'] == '594'):
					payload['T11'] = float(donnee['@valeur'][:-3])
				if (donnee['@donnee'] == '595'):
					payload['T12'] = float(donnee['@valeur'][:-3])
				if (donnee['@donnee'] == '618'):
                                        payload['C1'] = float(donnee['@valeur'][:-2])
				if (donnee['@donnee'] == '619'):
                                        payload['C2'] = float(donnee['@valeur'][:-2])
				if (donnee['@donnee'] == '614'):
                                        payload['C4'] = float(donnee['@valeur'][:-2])
				if (donnee['@donnee'] == '615'):
                                        payload['C5'] = float(donnee['@valeur'][:-2])
				if (donnee['@donnee'] == '616'):
                                        payload['C6'] = float(donnee['@valeur'][:-2])
				if (donnee['@donnee'] == '617'):
                                        payload['C7'] = float(donnee['@valeur'][:-2])



		print(payload)
		#client = mqtt_client.Client()

		client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2, client_id)
		client.on_connect = on_connect
		client.connect(broker, port)
		msg = str(payload).replace("'", '"')
		result = client.publish(topic, msg)
		# result: [0, 1]
		status = result[0]
		if status == 0:
    			print(f"Send `{msg}` to topic `{topic}`")
		else:
    			print(f"Failed to send message to topic {topic}")


loadCookie()

if (lecture_valeurs_donnees()==0):
	
	submit_form_and_get_cookie_solisart()
	lecture_valeurs_donnees()


