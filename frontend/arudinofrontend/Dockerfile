FROM node:alpine AS build

RUN apk update && apk add --no-cache make
RUN apk add make

WORKDIR /app

COPY package.json .

RUN npm install

COPY . .

RUN make prod

FROM nginx:alpine
COPY --from=build /app/dist /usr/share/nginx/html/