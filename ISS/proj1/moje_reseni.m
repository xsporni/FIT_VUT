%%%%_Vypracoval Alex Sporni(xsporn01@stud.fit.vutbr.cz)_%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_1_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
[signal, fs] = audioread('xsporn01.wav');
signal = signal';
dlzka_vzorku = length(signal);
fprintf('Vzorkovacia frekvencia signalu je: %.2f [Hz].\n', fs);
fprintf('Dlzka signalu vo vzorkách je: %.2f, v sekundách: %.2f [s].\n', dlzka_vzorku, dlzka_vzorku / fs);
sn = dlzka_vzorku / 16;
fprintf('Pocet reprezentovaných binárnych symbolov je: %.2f\n', sn);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_2_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
field = [];
count = 1; 
over = importdata('xsporn01.txt');
over = over';
for i = 1:dlzka_vzorku
match = mod(i,16);
    if match == 0
        if signal(i-8) > 0
        field(count) = 1;
        count = count + 1;
        else
        field(count) = 0;
        count = count + 1;
        end
    end
end
is_it = xor(field,over);
hold on
per = [1:320]/fs;
plot(per,signal(1:320));
stem(0.02/(20*2):0.02/20:0.02,field(1:20));
grid on; axis([0 0.02 -1 1]);
xlabel('t');
ylabel('s[n], symbols');
hold off

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_3_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
B = [0.0192 -0.0185 -0.0185 0.0192];
A = [1.0000 -2.8870 2.7997 -0.9113];
zplane (B,A);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_4_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
H = freqz(B,A,256); f=(0:255) / 256 * fs / 2; 
plot(221); plot (f,abs(H)); grid; xlabel('f[Hz]'); ylabel('PSD[dB]')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_5_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
ssn = filter(B,A,signal);
hold on
plot(per,signal(1:320));
plot(per,ssn(1:320));
xlabel('t');
ylabel('s[n], ss[n]');
hold off

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_6_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
shift = -17;
ssShifted = circshift(ssn,[shift],2);
per = [1:320]/fs;
field1 = [];
count1 = 1; 
for i = 1:dlzka_vzorku
match1 = mod(i,16);
    if match1 == 0
        if ssShifted(i-8) > 0
        field1(count1) = 1;
        count1 = count1 + 1;
        else
        field1(count1) = 0;
        count1 = count1 + 1;
        end
    end
end
hold on
plot(per,signal(1:320));
stem(0.02/(20*2):0.02/20:0.02,field1(1:20));
plot(per,ssn(1:320));
plot(per,ssShifted(1:320));
xlabel('t');
ylabel('s[n],ss[n],ssShifted[n], symbols');
hold off

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_7_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
odchylka = xor(field(1:length(field)),field1);
pocChyb = 0;
for i = 1:length(odchylka)
    if odchylka(i) == 1
        pocChyb = pocChyb + 1;
    end
end
chybovost = pocChyb / length(field) * 100;
fprintf('Chybovost je: %.2f percent\n', chybovost);
fprintf('Pocet chyb pri posune je: %d\n', pocChyb);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_8_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
frekvencia = (0 : dlzka_vzorku / 2 - 1) / dlzka_vzorku * fs;
fft_ssn = abs(fft(ssn));
fft_sn = abs(fft(signal));
fft_ssn = fft_ssn(1 : dlzka_vzorku / 2);
fft_sn = fft_sn(1 : dlzka_vzorku / 2);
hold on
plot(frekvencia, fft_sn);
plot(frekvencia, fft_ssn);
xlabel('f [Hz]');
ylabel('PSD[dB]');
grid;
hold off

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_9_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
x = hist(signal,100);
plot(x/dlzka_vzorku);
sum(x/dlzka_vzorku); %overenie suctu

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_10_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
koef = (-50 : 50);
R = xcorr(signal,'biased');
R = R(koef + dlzka_vzorku);
plot(koef, R);
grid;
xlabel('k');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_11_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fprintf('Hodnota koeficientu R[0] je %f.\n', R(51));
fprintf('Hodnota koeficientu R[1] je %f.\n', R(52));
fprintf('Hodnota koeficientu R[16] je %f.\n', R(67));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_12_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
N = length(signal);
L = 50;
x = linspace(min(signal), max(signal), 50);
h = zeros(L, L);
[~, ind1] = min(abs(repmat(signal(:)', L, 1) - repmat(x(:), 1, N)));
ind2 = ind1(1 + 1 : N);
for i = 1 : N - 1,
	d1 = ind1(i); d2 = ind2(i);
	h(d1, d2) = h(d1, d2) + 1;
end
surf = (x(2) - x(1)) ^ 2;
p = h / N / surf;
imagesc(x, x, p);
axis xy;
xlabel('x2');
ylabel('x1');
colorbar;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_13_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
is_it_legit = 0;
is_it_legit = sum(sum(p)) * surf;
fprintf('Sucet je %.2f takze sa jedna o spravu zdruzenu funkciu hustoty\n',is_it_legit);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%_14_%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fprintf('Hodnota koeficientu R[1] je %f. jedna sa o spravny vysledok\n', sum(sum(repmat(x(:), 1, L) .* repmat(x(:)', L, 1) .* p)) * surf);