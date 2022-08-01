[DDEV](https://github.com/drud/ddev) is a docker-based local development tool with excellent support for Backdrop CMS. Bee can be used in combination with DDEV by setting up a [custom DDEV command](https://ddev.readthedocs.io/en/stable/users/extend/custom-commands/).

1. Install `bee` within your `web-build` container by opening `.ddev/web-build/Dockerfile`. If this file does not exist yet, create it. Add the following contents:
```
ARG BASE_IMAGE
FROM $BASE_IMAGE

# Install Backdrop CLI tool bee:
RUN wget -qO bee.zip https://github.com/backdrop-contrib/bee/archive/1.x-1.x.zip
RUN unzip -q bee.zip && rm bee.zip
RUN mv bee-1.x-1.x /usr/local/bin/bee
```

2. Create a `bee` command by creating a new text file at `.ddev/commands/web/bee`. Put in the following contents:
```
#!/usr/bin/env bash

echo $DDEV_PRIMARY_URL;
/usr/local/bin/bee/bee.php --root=/var/www/html/$DDEV_DOCROOT --uri=$DDEV_PRIMARY_URL $@
```

3. In order for some commands to work, you should set a `$base_url` within your settings.ddev.php file:
```
$base_url = getenv('DDEV_PRIMARY_URL');
```

4. Restart the DDEV container:
```
ddev restart
```

5. Now you can use `bee` within the ddev web container:
```
ddev bee status
```