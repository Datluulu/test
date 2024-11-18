FROM nginx:1.23.0

# Create a non-root user and switch to that user
RUN useradd -r -u 1001 nginxuser
USER nginxuser

# Copy website content into the Nginx HTML directory
COPY . /usr/share/nginx/html/
