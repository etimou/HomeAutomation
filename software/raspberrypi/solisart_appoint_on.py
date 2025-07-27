import requests
import urllib3
import time
import xmltodict
import json
import base64



urllib3.disable_warnings()

session = requests.Session()

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


url = "secret"
form_data = {
    "id":"secret",
    "pass":"secret",
    "ihm":"admin",
    "connexion":"Se+connecter",
}

cookies = submit_form_and_get_cookie(url, form_data)

if cookies:
    print("Cookies récupérés :")
    print(cookies)

time.sleep(2)





data={"id":"secret", "xml":base64.b64encode(b'<valeurs><valeur donnee="150" valeur="Mg==" /><valeur donnee="151" valeur="Mg==" /><valeur donnee="156" valeur="MQ==" /></valeurs>')}
response = session.post("https://my.solisart.fr/admin/divers/ajax/ecriture_valeurs_donnees.php", data=data, verify=False)
print(response.text)







