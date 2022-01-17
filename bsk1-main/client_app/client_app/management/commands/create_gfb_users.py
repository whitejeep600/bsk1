from django.contrib.auth.models import User
from django.core.management import BaseCommand


class Command(BaseCommand):

    def handle(self, *args, **options):
        with open("/uzytkownicy.txt") as file:
            lines = file.read().splitlines()
            for line in lines:
                tokens = line.split(" ")
                if tokens[1] == "client":
                    user = User.objects.create_user(username=tokens[0],
                                                    email=tokens[0] + "@gfb.com",
                                                    password=tokens[0])
                    user.save()
