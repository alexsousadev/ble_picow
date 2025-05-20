

## SMP (Security Manager Protocol)
O SMP (Security Manager Protocol) é o cara da segurança no BLE. Ele é o responsável por negociar, validar e manter a segurança entre dois dispositivos que querem se conectar. Coisas como:
- Trocar chaves de criptografia
- Garantir que ninguém está espionando ([MITM protection](application))
- Armazenar as chaves para reconectar no futuro sem precisar parear de novo

O principal objetivo do SMP é garantir que a comunicação entre dispositivos Bluetooth seja autenticada (ou seja, os dispositivos são quem dizem ser) e criptografada (ou seja, a comunicação é protegida contra interceptação). 
> Com a chegada do Bluetooth 4.2, o pareamento utiliza como base um algoritmo chamado Elliptic Curve Diffie-Hellman (ECDH) para a troca de chaves (é feito com [criptografia assimétrica](https://www.alura.com.br/artigos/criptografia-diferencas-simetrica-assimetrica-homomorfica))

Os comandos SMP usam os serviços do [L2CAP](./l2cap.md) para realizar esses procedimentos.

---
## Próxima camada: [GAP](./gap.md)