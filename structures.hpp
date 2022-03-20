﻿// Solutionnaire du TD3 INF1015 hiver 2021
// Par Francois-R.Boyer@PolyMtl.ca
#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include <memory>
#include <functional>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "gsl/span"
using gsl::span;
using namespace std;

class Item; class Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.

class ListeFilms {
public:
	ListeFilms() = default;
	void ajouterFilm(Film* film);
	void enleverFilm(const Film* film);
	shared_ptr<Acteur> trouverActeur(const string& nomActeur) const;
	span<Film*> enSpan() const;
	int size() const { return nElements; }
	void detruire(bool possedeLesFilms = false);
	Film*& operator[] (int index) { return elements[index]; }
	Film* trouver(const function<bool(const Film&)>& critere) {
		for (auto& film : enSpan())
			if (critere(*film))
				return film;
		return nullptr;
	}

private:
	void changeDimension(int nouvelleCapacite);

	int capacite = 0, nElements = 0;
	Film** elements = nullptr; // Pointeur vers un tableau de Film*, chaque Film* pointant vers un Film.
};

template <typename T>
class Liste {
public:
	Liste() = default;
	explicit Liste(int capaciteInitiale) :  // explicit n'est pas matière à ce TD, mais c'est un cas où c'est bon de l'utiliser, pour ne pas qu'il construise implicitement une Liste à partir d'un entier, par exemple "maListe = 4;".
		capacite_(capaciteInitiale),
		elements_(make_unique<shared_ptr<T>[]>(capacite_))
	{
	}
	Liste(const Liste<T>& autre) :
		capacite_(autre.nElements_),
		nElements_(autre.nElements_),
		elements_(make_unique<shared_ptr<T>[]>(nElements_))
	{
		for (int i = 0; i < nElements_; ++i)
			elements_[i] = autre.elements_[i];
	}
	Liste(Liste<T>&&) = default;  // Pas nécessaire, mais puisque c'est si simple avec unique_ptr...
	Liste<T>& operator= (Liste<T>&&) noexcept = default;  // Utilisé pour l'initialisation dans lireFilm.

	void ajouter(shared_ptr<T> element)
	{
		assert(nElements_ < capacite_);  // Comme dans le TD précédent, on ne demande pas la réallocation pour ListeActeurs...
		elements_[nElements_++] = move(element);
	}

	// Noter que ces accesseurs const permettent de modifier les éléments; on pourrait vouloir des versions const qui retournent des const shared_ptr, et des versions non const qui retournent des shared_ptr.
	shared_ptr<T>& operator[] (int index) const { return elements_[index]; }
	span<shared_ptr<T>> enSpan() const { return span(elements_.get(), nElements_); }

private:
	int capacite_ = 0, nElements_ = 0;
	unique_ptr<shared_ptr<T>[]> elements_; // Pointeur vers un tableau de Acteur*, chaque Acteur* pointant vers un Acteur.
};
using ListeActeurs = Liste<Acteur>;

struct Acteur
{
	string nom; int anneeNaissance = 0; char sexe = '\0';
};
ostream& operator<< (ostream& os, const Acteur& acteur)
{
	return os << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}

class Affichable
{
public:
	virtual void afficher(ostream& o) const = 0;
};

class Item : public Affichable
{
public:
	std::string titre = "";
	int anneeSortie = 0;
	Item() {};
	Item(std::string titre, int annee) :titre(titre), anneeSortie(annee) {};
	virtual void afficher(ostream& o) const
	{
		o << "Titre: " << titre << endl;
		o << "  Annee de sortie: " << anneeSortie << endl;
	};
};

ostream& operator<< (ostream& os, const Affichable& affichable)
{
	affichable.afficher(os);
	return os;
}
class Livre : virtual public Item
{
public:
	std::string auteur = "";
	int ventes = 0;
	int pages = 0;
	Livre() :Item() {};
	Livre(std::string titre, int annee, std::string auteur, int ventes, int pages) :
		Item(titre, annee),
		auteur(auteur),
		ventes(ventes),
		pages(pages)
	{};
	Livre(std::string auteur, int ventes, int pages) :
		auteur(auteur),
		ventes(ventes),
		pages(pages)
	{};
	Livre(ifstream& fichier)
	{
		fichier >> quoted(titre);
		fichier >> anneeSortie;
		fichier >> quoted(auteur);
		fichier >> ventes;
		fichier >> pages;
		cout << "Création Livre " << titre << endl;
	}
	virtual void afficher(ostream& o) const
	{
		Item::afficher(o);
		o << "  Auteur: " << auteur << endl;
		o << "  Ventes: " << ventes << endl;
		o << "  Pages: " << pages << endl;
	};
};
class Film : virtual public Item
{
public:
	std::string realisateur = "";
	int recette = 0;
	ListeActeurs acteurs;

	Film():Item() {};
	Film(std::string titre, int anneeSortie, std::string realisateur, int recette, ListeActeurs acteurs) : Item(titre, anneeSortie), realisateur(realisateur), recette(recette), acteurs(acteurs) {};
	Film( std::string realisateur, int recette, ListeActeurs acteurs) : realisateur(realisateur), recette(recette), acteurs(acteurs) {};
	virtual void afficher(ostream& o) const
	{
		Item::afficher(o);
		o << "  Realisateur: " << realisateur << endl;
		o << "  Recette: " << recette << "M$" << endl;
		o << "Acteurs:" << endl;
		for (const shared_ptr<Acteur>& acteur : acteurs.enSpan())
			o << *acteur;
	};
};

class FilmLivre : public Film, public Livre
{
public:
	FilmLivre(const Film& film, const Livre& livre) : 
		Item(film.titre, film.anneeSortie),
		Livre(livre.auteur, livre.pages, livre.ventes), 
		Film(film.realisateur, film.recette, film.acteurs)
	{};
	virtual void afficher(ostream& o) const override
	{
		Item::afficher(o);
		o << "PARTIE FILM" << endl;
		o << "  Realisateur: " << realisateur << endl;
		o << "  Recette: " << recette << "M$" << endl;
		o << "Acteurs:" << endl;
		for (const shared_ptr<Acteur>& acteur : acteurs.enSpan())
			o << *acteur;
		o << "PARTIE LIVRE" << endl;
		o << "  Auteur: " << auteur << endl;
		o << "  Ventes: " << ventes << endl;
		o << "  Pages: " << pages << endl;
	};
};
